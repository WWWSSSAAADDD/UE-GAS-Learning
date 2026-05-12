---
title: "《InsideUE4》GamePlay架构（三）WorldContext，GameInstance，Engine"
source: "https://zhuanlan.zhihu.com/p/23167068"
author:
  - "[[大钊UE源码剖析，等编译的时候回答问题]]"
published:
created: 2026-04-18
description: "引言 前文提到说一个World管理多个Level，并负责它们的加载释放。那么，问题来了，一个游戏里是只有一个World吗？ WorldContext答案是否定的，首先World就不是只有一种类型，比如编辑器本身就也是一个World，里面…"
tags:
  - "clippings"
---
## 引言

前文提到说一个World管理多个Level，并负责它们的加载释放。那么，问题来了，一个游戏里是只有一个World吗？

## WorldContext

答案是否定的，首先World就不是只有一种类型，比如编辑器本身就也是一个World，里面显示的游戏场景也是一个World，这两个World互相协作构成了我们的编辑体验。然后点播放的时候，引擎又可以生成新的类型World来让我们测试。简单来说，UE其实是一个平行宇宙世界观。  
以下是一些世界类型：

```
namespace EWorldType
{
    enum Type
    {
        None,        // An untyped world, in most cases this will be the vestigial worlds of streamed in sub-levels
        Game,        // The game world
        Editor,        // A world being edited in the editor
        PIE,        // A Play In Editor world
        Preview,    // A preview world for an editor tool
        Inactive    // An editor world that was loaded but not currently being edited in the level editor
    };
}
```

而UE用来管理和跟踪这些World的工具就是WorldContext：  

![](https://pic4.zhimg.com/v2-5ded56be67f4082ee7f77a0c3fa0960f_1440w.png)

  
FWorldContext保存着ThisCurrentWorld来指向当前的World。而当需要从一个World切换到另一个World的时候（比如说当点击播放时，就是从Preview切换到PIE），FWorldContext就用来保存切换过程信息和目标World上下文信息。所以一般在切换的时候，比如OpenLevel，也都会需要传FWorldContext的参数。一般就来说，对于独立运行的游戏，WorldContext只有唯一个。而对于编辑器模式，则是一个WorldContext给编辑器，一个WorldContext给PIE（Play In Editor）的World。一般来说我们不需要直接操作到这个类，引擎内部已经处理好各种World的协作。  
不仅如此，同时FWorldContext还保存着World里Level切换的上下文：

```cpp
struct FWorldContext
{
    [...]
    TEnumAsByte<EWorldType::Type>    WorldType;

    FSeamlessTravelHandler SeamlessTravelHandler;

    FName ContextHandle;

    /** URL to travel to for pending client connect */
    FString TravelURL;

    /** TravelType for pending client connects */
    uint8 TravelType;

    /** URL the last time we traveled */
    UPROPERTY()
    struct FURL LastURL;

    /** last server we connected to (for "reconnect" command) */
    UPROPERTY()
    struct FURL LastRemoteURL;

}
```

这里的TravelURL和TravelType就是负责设定下一个Level的目标和转换过程。  

```
// Traveling from server to server.
UENUM()
enum ETravelType
{
    /** Absolute URL. */
    TRAVEL_Absolute,
    /** Partial (carry name, reset server). */
    TRAVEL_Partial,
    /** Relative URL. */
    TRAVEL_Relative,
    TRAVEL_MAX,
};

void UEngine::SetClientTravel( UWorld *InWorld, const TCHAR* NextURL, ETravelType InTravelType )
{
    FWorldContext &Context = GetWorldContextFromWorldChecked(InWorld);
    // set TravelURL.  Will be processed safely on the next tick in UGameEngine::Tick().
    Context.TravelURL    = NextURL;
    Context.TravelType   = InTravelType;
    [...]
}
```

粗略的流程是UE在OpenLevel的时候， 先设置当前World的Context上的TravelURL，然后在UEngine::TickWorldTravel的时候判断TravelURL非空来真正执行Level的切换。具体的Level切换详细流程比较复杂，目前先从大局上理解整体结构。总而言之，WorldContext既负责World之间切换的上下文，也负责Level之间切换的操作信息。  

**思考：为何Level的切换信息不放在World里？**  
因为UE有一个逻辑，一个World只有一个PersistentLevel（见上篇），而当我们OpenLevel一个PersistentLevel的时候，实际上引擎做的是先释放掉当前的World，然后再创建个新的World。所以如果我们把下一个Level的信息放在当前的World中，就不得不在释放当前World前又拷贝回来一遍了。  
而LoadStreamLevel的时候，就只是在当前的World中载入对象了，所以其实就没有这个限制了。

```cpp
void UGameplayStatics::LoadStreamLevel(UObject* WorldContextObject, FName LevelName,bool bMakeVisibleAfterLoad,bool bShouldBlockOnLoad,FLatentActionInfo LatentInfo)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject))
    {
        FLatentActionManager& LatentManager = World->GetLatentActionManager();
        if (LatentManager.FindExistingAction<FStreamLevelAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
        {
            FStreamLevelAction* NewAction = new FStreamLevelAction(true, LevelName, bMakeVisibleAfterLoad, bShouldBlockOnLoad, LatentInfo, World);
            LatentManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
        }
    }
}
```

World->GetLatentActionManager()其实也算是保存在当前World里了。  

**思考：为何World和Level的切换要放在下一帧再执行？**  
首先Level的加载显然是比较慢的，需要载入Map，相应的Mesh，Material……等等。所以这个操作就必须异步化，异步的话其实就剩下两种方式，一种是先记录下来信息之后再执行；一种是命令模式立马往队列里压个命令之后再执行。注意，因为OpenLevel还要相应在主线程生成相应Actor对象，所以有些部分还是要在主线程完成的。这两种模式其实都可以达成需求，前者更加简单明了，后者相对统一。UE也是个进化过来的引擎，也并不是所有的代码都完美无缺。猜想其实也是一开始这么简单就这么做了，后来也没有特别大的改动的动力就一直这样了。引擎最终比的是生产效率的提高，确实也不是代码有多优雅。

## GameInstance

那么这些WorldContexts又是保存在哪里的呢？追根溯源：  

![](https://pic2.zhimg.com/v2-36b45a7b36ac77d978719bc6fe8db17b_1440w.png)

  
GameInstance里会保存着当前的WorldConext和其他整个游戏的信息。明白了GameInstance是比World更高的层次之后，我们也就能明白为何那些独立于Level的逻辑或数据要在GameInstance中存储了。  
这一点其实也很好理解，大凡游戏引擎都会有一个Game的概念，不管是叫Application还是Director，它都是玩家能直接接触到的最根源的操作类。而UE的GameInstance因为继承于UObject，所以就拥有了动态创建的能力，所以我们可以通过指定GameInstanceClass来让UE创建使用我们自定义的GameInstance子类。所以不论是C++还是BP，我们通常会继承于GameInstance，然后在里面编写应用于整个游戏范围的逻辑。  
因为经常有初学者会问到：我的Level切换了，变量数据就丟了，我应该把那些数据放在哪？再清晰直白一点，GameInstance就是你不管Level怎么切换，还是会一直存在的那个对象！

## Engine

让我们继续再往上，终于得见UE大神：  

![](https://pica.zhimg.com/v2-94d1f4e3750b6f4fd09d02b20bc980b0_1440w.png)

此处UEngine分化出了两个子类：UGameEngine和 [UEditorEngine](https://zhida.zhihu.com/search?content_id=1452434&content_type=Article&match_order=1&q=UEditorEngine&zhida_source=entity) 。众所周知，UE的编辑器也是UE用自己的引擎渲染出来的，采用的也是Slate那套UI框架。好处有很多，比如跨平台比较统一，UI框架可以复用一套控件库，Dogfood等等，此处不再细讲。所以本质上来说，UE的编辑器其实也是个游戏！我们是在编辑器这个游戏里面创造我们自己的另一个游戏。话虽如此，但比较编辑器和游戏还是有一定差别的，所以UE会在不同模式下根据编译环境而采用不同的具体Engine类，而在基类UEngine里通过一个WorldList保存了所有的World。

- Standalone Game：会使用UGameEngine来创建出唯一的一个GameWorld，因为也只有一个，所以为了方便起见，就直接保存了GameInstance指针。
- 而对于编辑器来说，EditorWorld其实只是用来预览，所以并不拥有OwningGameInstance，而PlayWorld里的OwningGameInstance才是间接保存了GameInstance.

目前来说，因为UE还不支持同时运行多个World（当前只能一个，但可以切换），所以GameInstance其实也是唯一的。提前说些题外话，虽然目前网络部分还没涉及到，但是当我们在Editor里进行MultiplePlayer的测试时，每一个Player Window里都是一个World。如果是DedicateServer模式，那DedicateServer也会是一个World。  
最后实例化出来的UEngine实例用一个全局的GEngine变量来保存。至此，我们已经到了引擎的最根处:

```cpp
//UnrealEngine\Engine\Source\Runtime\Engine\Private\UnrealEngine.cpp
ENGINE_API UEngine*    GEngine = NULL;
```

GEngine可以说是一切开始的地方了。翻看引擎源码，到处也可以看见从GEngine->出来的引用。  

## GamePlayStatics

既然我们在引擎内部C++层次已经有了访问World操作Level的能力，那么在暴露出的蓝图系统里，UE为了我们的使用方便，也在Engine层次为我们提供了便利操作蓝图函数库。

```
UCLASS ()
class UGameplayStatics : public UBlueprintFunctionLibrary
```

我们在蓝图里见到的GetPlayerController、SpawActor和OpenLevel等都是来至于这个类的接口。这个类比较简单，相当于一个C++的静态类，只为蓝图暴露提供了一些静态方法。在想借鉴或者是查询某个功能的实现时，此处往往会是一个入口。  

## 总结

从结构上而言，我们已经来到了最根源的地方。GEngine仿佛就是一棵大树的根，当我们拎起它的时候，也会带出整个游戏世界的各个对象。但目前这些对象：Object->Actor+Component->Level->World->WorldContext->GameInstance->Engine，确实已经足够表达UE游戏世界的各个部分。  
那作为GamePlay部分而言，我们还有一个问题：UE是如何把在该对象结构上表达游戏逻辑的？  
如果说：“程序=数据+算法”的话，那UE的GamePlay我们已经讨论完了数据部分，而下篇我们将开始讨论UE的游戏逻辑“算法”部分。

上篇： [《InsideUE4》GamePlay架构（二）Level和World](https://zhuanlan.zhihu.com/p/22924838)

下篇： [《InsideUE4》GamePlay架构（四）Pawn](https://zhuanlan.zhihu.com/p/23321666?refer=insideue4)

*UE4.14*

\---------------------------------------------------------------------------------------------------------------------------

知乎专栏： [InsideUE4](https://zhuanlan.zhihu.com/insideue4)

UE4深入学习QQ群： **456247757** (非新手入门群，请先学习完官方文档和视频教程)

微信公众号： **aboutue** ，关于UE的一切新闻资讯、技巧问答、文章发布，欢迎关注。

**个人原创，未经授权，谢绝转载！**

3 人已送礼物

编辑于 2021-11-19 10:54[游戏引擎](https://www.zhihu.com/topic/19556258)[虚幻引擎](https://www.zhihu.com/topic/19824201)[游戏开发](https://www.zhihu.com/topic/19553361)

---

<!-- AI注释-新增开始：以下内容为教学增补，不属于原始文章正文 -->
## [AI注释-新增] 三步学习法拆解与验点（K1-K8）

> [AI注释说明]
> 1. 本区为 AI 新增教学内容，原文正文保持不变。
> 2. 每次只推进 1 个 K 点，当前点未通过验证前不得进入下一点。
> 3. 每个 K 点都包含：核心句、为什么重要、机制解释、思考题、考察重点、参考答案。

### [AI注释-新增] K点总览

| 编号 | 对应原文章节 | 核心句 | 前置条件 |
| --- | --- | --- | --- |
| K1 | 引言 | WorldContext 是多世界并存与切换的中枢。 | 无 |
| K2 | WorldContext | WorldType 区分了编辑、游戏、PIE 等不同世界语义。 | K1 |
| K3 | WorldContext | TravelURL/TravelType 负责把“切换意图”延后到合适时机执行。 | K1-K2 |
| K4 | WorldContext | Level 切换信息不放在 World 里，是为了避免释放时丢失上下文。 | K3 |
| K5 | GameInstance | GameInstance 是比 World 更高一级的长期存在容器。 | K1-K4 |
| K6 | Engine | UEngine 在编辑器和游戏模式下分化出不同运行根。 | K5 |
| K7 | Engine | GEngine 是整个 UE 运行时最核心的全局入口。 | K6 |
| K8 | GameplayStatics | UGameplayStatics 把底层世界操作封装成蓝图友好静态接口。 | K5-K7 |

### [AI注释-新增][K1] 为什么需要 WorldContext

- 核心句：WorldContext 负责跟踪“当前是什么世界”以及“如何从一个世界切到另一个世界”。
- 为什么重要：没有上下文容器，编辑器世界、PIE 世界和游戏世界会互相干扰。
- 机制解释：
- L1 直觉层：你需要一个“场景切换登记本”，记录当前在哪个世界。
- L2 机制层：WorldContext 保存当前 World、切换状态和相关流程信息。
- L3 工程层：OpenLevel、PIE、编辑器预览都依赖这层上下文协调。
- 思考题：如果没有 WorldContext，编辑器预览和 PIE 切换时最容易出什么问题？
- 考察重点：是否能说清“WorldContext 管的是切换过程，而不是单一世界内容”。
- 参考答案：最容易出问题的是世界切换状态丢失、上下文混淆和切换后引用失效，因为没有独立容器保存当前世界与目标世界信息。

### [AI注释-新增][K2] WorldType 的意义

- 核心句：WorldType 是把“同一个 World 概念”按用途细分的语义标签。
- 为什么重要：不同世界类型对应不同运行行为和工具链处理方式。
- 机制解释：
- L1 直觉层：同样叫“世界”，但有的是编辑用，有的是游戏用。
- L2 机制层：EWorldType 区分 Editor、Game、PIE、Preview 等场景。
- L3 工程层：引擎内部会根据类型选择不同的协作和切换逻辑。
- 思考题：为什么 Editor 世界和 PIE 世界不能简单当作同一种 World 处理？
- 考察重点：是否能联系编辑器协作、预览和运行时行为差异。
- 参考答案：因为它们承担的任务不同，Editor 世界用于编辑和预览，PIE 世界用于运行测试，二者的生命周期、输入和切换规则都不同。

### [AI注释-新增][K3] TravelURL 与 TravelType 为什么要延后执行

- 核心句：切关卡不是立刻执行，而是先记录意图，再在合适的 Tick 阶段处理。
- 为什么重要：Level 加载和销毁是重操作，不能直接在任意时刻同步完成。
- 机制解释：
- L1 直觉层：先记下“要去哪”，等车开到合适时机再出发。
- L2 机制层：SetClientTravel 只是设置上下文里的 TravelURL/TravelType。
- L3 工程层：UEngine::TickWorldTravel 在后续 Tick 中真正执行切换。
- 思考题：为什么 OpenLevel 不能把切换逻辑直接写成一次同步函数完成？
- 考察重点：是否理解延后执行是为了异步加载、安全切换和主线程协调。
- 参考答案：因为关卡加载耗时大，而且对象创建和销毁需要在合适的线程与时机完成，直接同步切换会导致卡顿并增加状态错误风险。

### [AI注释-新增][K4] 为什么 Level 切换信息不放在 World 里

- 核心句：World 释放时会丢失自身信息，所以切换意图必须放在更高层的上下文里保存。
- 为什么重要：这解释了 WorldContext 存在的根本原因之一。
- 机制解释：
- L1 直觉层：如果房子要拆，搬家清单不能放在房子里。
- L2 机制层：OpenLevel 会释放当前 World，再创建新 World。
- L3 工程层：把旅行信息放在 WorldContext 能保证跨 World 的切换状态不丢失。
- 思考题：如果把 TravelURL 存在当前 World 中，会在哪一步丢失？
- 考察重点：能否说出“当前 World 被释放”这一关键原因。
- 参考答案：会在当前 World 被释放时丢失，因为切关卡的过程本身就会销毁旧 World，放在旧 World 中的数据无法继续保存到新 World。

### [AI注释-新增][K5] GameInstance 的地位

- 核心句：GameInstance 是比 World 更长生命周期的游戏级容器。
- 为什么重要：跨关卡保留的数据和逻辑，通常都应放在 GameInstance。
- 机制解释：
- L1 直觉层：它像玩家账号的临时总仓库，换地图也不丢。
- L2 机制层：GameInstance 持有当前 WorldContext 和全局游戏信息。
- L3 工程层：Level 切换后仍要保存的状态，不适合放在 World 里。
- 思考题：哪些数据适合放在 GameInstance，哪些不适合？
- 考察重点：是否能区分“跨关卡常驻数据”和“关卡局部数据”。
- 参考答案：适合放玩家进度、全局设置、会话信息等；不适合放关卡临时实体、局部机关状态等，因为这些应随关卡加载释放而变化。

### [AI注释-新增][K6] Engine 为什么要分成 Game 与 Editor 两类

- 核心句：UEngine 在不同工作模式下会承载不同的运行根，编辑器和游戏逻辑并不完全相同。
- 为什么重要：这决定了引擎初始化、窗口行为和世界列表的管理方式。
- 机制解释：
- L1 直觉层：编辑器本身也是一个“运行中的程序”，但用途和游戏不同。
- L2 机制层：UGameEngine 主要服务游戏运行，UEditorEngine 主要服务编辑与预览。
- L3 工程层：不同 Engine 子类会决定 WorldList、GameInstance 等对象如何组织。
- 思考题：为什么说 UE 编辑器本身“也是一个游戏”？
- 考察重点：是否理解编辑器也是用 UE 自己的引擎和 UI 系统渲染出来的。
- 参考答案：因为编辑器本身也是由引擎运行、渲染并管理世界状态的程序，只是其目标是编辑和预览而不是对外发布的最终游戏。

### [AI注释-新增][K7] GEngine 的作用

- 核心句：GEngine 是 UE 运行时最核心的全局入口，很多系统都会从这里出发。
- 为什么重要：理解 GEngine 有助于你建立“引擎根对象”的坐标系。
- 机制解释：
- L1 直觉层：像总开关，很多线路都从它分出去。
- L2 机制层：UEngine 实例被全局 GEngine 指针持有。
- L3 工程层：源码里大量逻辑都会通过 GEngine 访问 World、GameInstance 或工具接口。
- 思考题：为什么全局入口对象会在阅读源码时频繁出现？
- 考察重点：是否能说出“全局协调中心”的角色。
- 参考答案：因为它是统一管理世界、游戏实例和编辑器模式的总入口，许多子系统都需要从这里获取上下文和服务。

### [AI注释-新增][K8] UGameplayStatics 为什么重要

- 核心句：UGameplayStatics 把底层世界操作包装成蓝图和脚本更容易使用的静态接口。
- 为什么重要：它是蓝图用户和底层引擎能力之间的重要桥梁。
- 机制解释：
- L1 直觉层：把复杂操作做成“随手就能调”的工具函数。
- L2 机制层：GetPlayerController、SpawnActor、OpenLevel 等常用功能集中在这个库里。
- L3 工程层：这让上层逻辑能不直接接触底层对象组织细节。
- 思考题：为什么把这些功能做成静态工具函数，而不是要求用户总是手动找 World/Engine？
- 考察重点：是否理解“降低调用门槛”和“保持接口统一”。
- 参考答案：因为这些操作很常用，做成静态库函数能减少样板代码，避免每次都手动查找上下文对象，也更符合蓝图的调用习惯。

### [AI注释-新增] 单篇文章通过规则（执行版）

1. 每个 K 点按 0-5 分评分，低于 3 分视为未通过。
2. 未通过点必须补做验证任务并补交证据。
3. 全文通过条件：K 点通过率 >= 80%，且关键点 K1/K5/K7 均 >= 3 分。
4. 若连续 2 个点未通过，回退到上一个通过点重做 L1-L2 讲解。

### [AI注释-新增] 自测记录模板

| 日期 | 文章 | K点 | 得分(0-5) | 证据摘要 | 结论 | 下一步 |
| --- | --- | --- | --- | --- | --- | --- |
| YYYY-MM-DD | GamePlay架构（三）WorldContext，GameInstance，Engine | K1 | 0-5 | 例如：WorldContext职责对照表 | 通过/未通过 | 进入K2或补测 |

<!-- AI注释-新增结束 -->