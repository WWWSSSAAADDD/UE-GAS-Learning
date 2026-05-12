---
title: "《InsideUE4》GamePlay架构（二）Level和World"
source: "https://zhuanlan.zhihu.com/p/22924838"
author:
  - "[[大钊UE源码剖析，等编译的时候回答问题]]"
published:
created: 2026-04-18
description: "引言 上文谈到Actor和Component的关系，UE利用Actor的概念组成一片游戏对象森林，并利用Component组装扩展Actor的能力，让世界里拥有了形形色色的Actor们，拥有了自由表达3D世界的能力。 那么，这些Actor们，到底…"
tags:
  - "clippings"
---
## 引言

上文谈到Actor和Component的关系，UE利用Actor的概念组成一片游戏对象森林，并利用Component组装扩展Actor的能力，让世界里拥有了形形色色的Actor们，拥有了自由表达3D世界的能力。  
那么，这些Actor们，到底是怎么组织起来的呢？

既然提到了世界，我们的直觉反应是采用一个"World"对象来包容所有的Actor们。但是当游戏的虚拟世界非常巨大时，这种方式就捉襟见肘了。首先，目前虽然PC的性能日益强大，但是依然内存也限制了不能一下子加载进所有的游戏资源；其次，因为玩家的活动和可见范围有限，为了最优性能，把即使是很远的跟玩家无关的对象也考虑进来也明显是不明智的。所以我们需要一种更细粒度的概念来划分世界。

不同的游戏引擎们，看待这个过程的角度和理念也不一样。 [Cocos2dx](https://zhida.zhihu.com/search?content_id=1355212&content_type=Article&match_order=1&q=Cocos2dx&zhida_source=entity) 会认为游戏世界是由Scene组成的，Scene再由一个个Layer层叠表现，然后再有一个Director来导演整个游戏。 [Unity](https://zhida.zhihu.com/search?content_id=1355212&content_type=Article&match_order=1&q=Unity&zhida_source=entity) 觉得世界也是由Scene组成的，然后一个Application来扮演上帝来LoadLevel，后来换成了SceneManager。其他的，有的会称为关卡（Level）或地图（map）等等。而UE中把这种拆分叫做关卡（Level），由一个或多个Level组成一个World。  
不要觉得这种划分好像很随意，只是个名字不同而已。实际上一个游戏引擎的“世界观”关系到了一整串后续的内容组织，玩家的管理，世界的生成，变换和毁灭。游戏引擎内部的资源的加载释放也往往都是和这种划分（Level）绑定在一起的。

## Level

在UE的世界中，我们之前已经有了空气（C++）,土壤（UObject），物件（Actor）。而现在UE又施展神力创建了一片片大陆（Level），在这片大陆上（.map文件），Actor们秩序井然，各种地形拔地而起，植被繁茂，天空雾云缭绕，圣光普照，这也是玩家们降生开始精彩冒险的地方。  

![](https://pic1.zhimg.com/v2-bca44e1f846c37b12f08bc0a6659b4ae_1440w.png)

可以从ULevel的前缀U看出来Level（大陆）也确实是继承于UObject（土壤）的。那既然同属于Object下面的各Actor们都拥有了一定的智能能力（支持蓝图脚本），Level自然也得体现出大地的意志，所以默认带了一个土地公（ALevelScriptActor），允许我们在关卡里编写脚本，可以对本关卡里的所有Actor通过名字呼之则来，关卡蓝图实际上就代表着该片大陆上的运行规则。  
在Level已经有了管理者之后，一开始大家都挺满意，但渐渐的就发现，好像各个Level需要的功能好像都差不多，都是修改一下光照，物理等一些属性。所以为了方便起见，UE便给每一个Level也都默认配了一个书记官（Info），他一一记录着本Level的各种规则属性，在UE需要的时候便负责相告。更重要的是，在Level需要有其他管理人员一起协助的时候，他也记录着“游戏模式”的名字来让UE可以指派。  
前面我们说过，有一些Actor是不“显示”的（没有SceneComponent），是不能“摆放”到Level里的，但是它依然可以在关卡里出力。其中一个家族系列就是AInfo和其之类。今天我们只简单介绍一下跟Level直接相关的一位书记官：AWorldSettings。  

![](https://pica.zhimg.com/v2-570955742351c933a4bc3cdf822830f2_1440w.jpg)

其实虽然名字叫做WorldSettings，但其实只是跟Level相关，我猜可能是在上古时代，当时整个世界只有一块大陆，人们就以为当前的大陆就是整个世界，所以给这块大陆的设置就起名为WorldSettings，后来等技术进步了，发现必须有其他大陆了，这个名字已经用得太多反而不好改了，就只好遗留下来了。当然也有可能是因为当Level被添加进World后，这个Level的Settings如果是主PersistentLevel，那它就会被当作整个World的WorldSettings。  
注意，Actors里也保存着AWorldSettings和ALevelScriptActor的指针，所以Actors实际上确实是保存了所有Actor。

**思考：为何AWorldSettings要放进在Actors\[0\]的位置？而ALevelScriptActor却不用？**

```cpp
void ULevel::SortActorList()
{
    //[...]
    TArray<AActor*> NewActors;
    TArray<AActor*> NewNetActors;
    NewActors.Reserve(Actors.Num());
    NewNetActors.Reserve(Actors.Num());
    // The WorldSettings tries to stay at index 0
    NewActors.Add(WorldSettings);
    // Add non-net actors to the NewActors immediately, cache off the net actors to Append after
    for (AActor* Actor : Actors)
    {
        if (Actor != nullptr && Actor != WorldSettings && !Actor->IsPendingKill())
        {
            if (IsNetActor(Actor))
            {
                NewNetActors.Add(Actor);
            }
            else
            {
                NewActors.Add(Actor);
            }
        }
    }
    iFirstNetRelevantActor = NewActors.Num();
    NewActors.Append(MoveTemp(NewNetActors));
    Actors = MoveTemp(NewActors);   // Replace with sorted list.
    // Add all network actors to the owning world
    //[...]
}
```

实际上通过这一段代码可知，Actors们的排序依据是把那些“非网络”的Actor放在前面，而把“网络可复制”的Actor们放在后面，然后加一个起始索引标记iFirstNetRelevantActor，相当于为网络Actor划分了一个缓存，从而加速了网络复制时的检测速度。AWorldSettings因为都是静态的数据提供者，在游戏运行过程中也不会改变，不需要网络复制，所以也就可以一直放在前列，而如果再加个规则，一直放在第一个的话，也能同时把AWorldSettings和其他的前列Actor们再度区分开，在需要的时候也能加速判断。ALevelScriptActor因为是代表关卡蓝图，是允许携带“复制”变量函数的，所以也有可能被排序到后列。

**思考：既然ALevelScriptActor也继承于AActor,为何关卡蓝图不设计能添加Component？**  
观察到，平常我们在创建Actor的时候，我们蓝图界面是可以创建Component的。  
那为什么在关卡蓝图里，却不能这么做（没有提供该界面功能）？  
我虽然在图里标出了Level中拥有ModelComponents，但那其实只是针对BSP应用的一个子集。通过源码发现，其实UE自己也是在C++里往ALevelScriptActor添加UInputComponent来实现关卡蓝图可以响应事件。

```cpp
void ALevelScriptActor::PreInitializeComponents()
{
    if (UInputDelegateBinding::SupportsInputDelegate(GetClass()))
    {
        // create an InputComponent object so that the level script actor can bind key events
        InputComponent = NewObject<UInputComponent>(this);
        InputComponent->RegisterComponent();
        UInputDelegateBinding::BindInputDelegates(GetClass(), InputComponent);
    }
    Super::PreInitializeComponents();
}
```

其实既然ALevelScriptActor是个Actor，那意味着我们当然可以为它添加组件，实际上也确实可以这么做。比如你可以在关卡蓝图里这么干：  

![](https://picx.zhimg.com/v2-4c50c55332f38aa13a223c64c0da85f3_1440w.png)

而如果你实际意识到关卡蓝图本身就是一个看不见的Actor，你就可以在上面用Actor的各种操作：  

![](https://pica.zhimg.com/v2-39656dc3a07e43f671c0c8eed3779846_1440w.png)

在关卡蓝图里的self其实也是个Actor！虽然一般这么干也没什么毛用。  
那么好好想想，为啥UE要给你这么一个关卡蓝图界面呢？  
  
在此，我也只能进行一番猜测，ALevelScriptActor作为一个特化的Actor,却把Components列表界面给隐藏了，说明UE其实是不希望我们去复杂化关卡构成的。  
假设说UE开放了关卡Component，那么我们在创建组件时就必然要考虑一个问题：哪些是ActorComponent，哪些是LevelComponent，再怎么ALevelScriptActor本质是个Actor，但Level的概念还是要突出，ALevelScriptActor的Actor本质是要隐藏的。所以用户就会多一些心智负担，可能混淆。而如果像这样不开放，大家的思路就都转向先创建个Actor，然后再往之上添加component，思路会比较统一清晰。  
再之，从游戏逻辑的组织上来说，Level其实更应该表现为一个Actor的容器。UE其实也是不鼓励在Level里编写太复杂的逻辑的。所以才接着会有了之后的 [GameMode](https://zhida.zhihu.com/search?content_id=1355212&content_type=Article&match_order=1&q=GameMode&zhida_source=entity),Controller那些真正的逻辑控制类（后续会再细讨论）。  
所以游戏引擎也并不是说最大化的暴露一切功能给你就是最好的，有时候选择太多了反而容易出错。在这一点上，我觉得UE很好的保持了克制，为我们提供了一个优秀的清晰的不易出错的框架，同时也对高阶用户保留了灵活性。

## World

终于，到了把大陆们（Level）拼装起来的时候了。可以用SubLevel的方式：

![](https://pica.zhimg.com/v2-dfd6bc119d32cc4f9f958b682bd0d480_1440w.png)

也支持WorldComposition的方式自动把项目里的所有Level都组合起来，并设置摆放位置：

![](https://pic3.zhimg.com/v2-9119eecdae3bebffc8e306f41995a68c_1440w.jpg)

具体摆放的操作和技巧并不是本文的重点。简单本质来说，就是一个World里有多个Level，这些Level在什么位置，是在一开始就加载进来，还是Streaming运行时加载。  
UE里每个World支持一个PersistentLevel和多个其他Level：  

![](https://pic4.zhimg.com/v2-41963e6f39bcefb2d799d31bec703759_1440w.png)

Persistent的意思是一开始就加载进World，Streaming是后续动态加载的意思。Levels里保存有所有的当前已经加载的Level，StreamingLevels保存整个World的Levels配置列表。PersistentLevel和CurrentLevel只是个快速引用。在编辑器里编辑的时候，CurrentLevel可以指向其他Level，但运行时CurrentLevel只能是指向PersistentLevel。

**思考：为何要有主PersistentLevel？**  
首先，World至少得有一个Level，就像你也得先出生在一块大陆上才可以继续谈起去探索别的新大陆。所以这块玩家出生的大陆就是主Level了。当然了，因为我们也可以同时配置别的Level一开始就加载进来，其实跟PersistentLevel是差不多等价的，但再考虑到另一问题：Levels拼接进World一起之后，各自有各自的worldsetting，那整个World的配置应该以谁的为主？

```cpp
AWorldSettings* UWorld::GetWorldSettings( bool bCheckStreamingPesistent, bool bChecked ) const
{
    checkSlow(IsInGameThread());
    AWorldSettings* WorldSettings = nullptr;
    if (PersistentLevel)
    {
        WorldSettings = PersistentLevel->GetWorldSettings(bChecked);
        if( bCheckStreamingPesistent )
        {
            if( StreamingLevels.Num() > 0 &&
                StreamingLevels[0] &&
                StreamingLevels[0]->IsA<ULevelStreamingPersistent>()) 
            {
                ULevel* Level = StreamingLevels[0]->GetLoadedLevel();
                if (Level != nullptr)
                {
                    WorldSettings = Level->GetWorldSettings();
                }
            }
        }
    }
    return WorldSettings;
}
```

可以看出，World的Settings也是以PersistentLevel为主的，但这也并不意味着其他Level的Settings就完全没有作用了，本篇也无法一一列出所有配置选项来说明，简单来说，就是需要在整个世界范围内起作用的配置选项（比如VR的WorldToMeters，KillZ，WorldGravity其他大部分都是）就是需要从主PersistentLevel的配置中提取。而一些配置选项可以在单独Level中起作用的，比如在编辑Level时的光照质量配置就是一个个Level单独的，目前这种配置很少，但可能以后也会增加。在这里只是阐明一个为主其他为辅的Level配置系统。

**思考：Levels们的Actors和World有直接关系吗？**  
当别的Level被添加进当前World之后，我们能直接在WorldOutliner里看到其他Level的Actor们。  

![](https://pic1.zhimg.com/v2-2b8060bff22a403eb19bf1efb191b1da_1440w.png)

但这并不代表着World直接引用了Level里的Actor们。TActorIteratorBase（World的Actor迭代器）内部的实现也只是在遍历Levels来获得所有Actor。当然World为了更快速的操作Controllers和Pawn也都保存了引用。但Levels却共享着World的一个PhysicsScene，这也意味着Levels里的Actors的物理实体其实都是在World里的，这也好理解，毕竟物理的碰撞之类的当然要是全局的了。再说到导航，World在拼接Level的时候，也是会同时把两个Level的导航网格给“拼接”起来的。当然目前还不是深入细节的时候，现在只要从大局上明白World-Level-Actor的关系。

**思考：为什么要在Level里保存Actors，而不是把所有Map的Actors配置都生成在World一个总Actors里？**  
这肯定也是一种实现方式，好处是把整个World看成一个整体，所有的actors都从属于world，这样就不存在Level边界，可以更整体的处理Actors的作用范围和判定问题，实现上也少了拼接导航等步骤。当然坏处也是模糊了Level边界，这样在加载进一个Level之后，之后再动态释放，就需要再重新再从整体中抽离出部分来释放，这个筛选过程也会产生比较大的损耗。试着去理解UE的权衡，应该是尽量的把损耗平摊（这里是把Level加载释放的损耗尽量减小），才不会产生比较大的帧率波动，让玩家感觉到卡帧。

## 总结

Level作为Actor的容器，同时也划分了World，一方面支持了Level的动态加载，另一方面也允许了团队的实时协作，大家可以同时并行编辑不同的Level。一般而言，一个玩家从游戏开始到结束，UE会创造一个GameWorld给玩家并一直存在。玩家切换场景或关卡，也只是在这个World中加载释放不同的Level。既然Level拥有了管理者（LevelScriptActor），玩家可以编写特定关卡的逻辑，那么我们能否对World这种层次编写逻辑呢？答案是肯定的，不过本文篇幅有限，敬请期待下篇。

上篇： [《Inside UE4》GamePlay架构（一）Actor和Component](https://zhuanlan.zhihu.com/p/22833151)

下篇： [《InsideUE4》GamePlayer架构（三）WorldContext，GameInstance，Engine](https://zhuanlan.zhihu.com/p/23167068)

*UE4.14*

\---------------------------------------------------------------------------------------------------------------------------

知乎专栏： [InsideUE4](https://zhuanlan.zhihu.com/insideue4)

UE4深入学习QQ群： **456247757** (非新手入门群，请先学习完官方文档和视频教程)

微信公众号： **aboutue** ，关于UE的一切新闻资讯、技巧问答、文章发布，欢迎关注。

**个人原创，未经授权，谢绝转载！**

4 人已送礼物

编辑于 2022-07-06 23:34[游戏引擎](https://www.zhihu.com/topic/19556258)[虚幻引擎](https://www.zhihu.com/topic/19824201)[游戏开发](https://www.zhihu.com/topic/19553361)

---

<!-- AI注释-新增开始：以下内容为教学增补，不属于原始文章正文 -->
## [AI注释-新增] 三步学习法拆解与验点（K1-K10）

> [AI注释说明]
> 1. 本区为 AI 新增教学讲解，原文正文未做任何删改。
> 2. 学习必须按 K1 -> K10 递进，当前点未过关不得进入下一点。
> 3. 每个 K 点包含：核心句、重要性、L1/L2/L3、最小验证任务、证据类型、过关标准。

### [AI注释-新增] K点总览

| 编号 | 对应原文章节 | 核心句 | 前置条件 |
| --- | --- | --- | --- |
| K1 | 引言 | 世界组织的核心是分层，而不是把所有 Actor 塞进一个大容器。 | 无 |
| K2 | Level | Level 是“大陆”层，负责承载局部对象集合与关卡脚本。 | K1 |
| K3 | Level | WorldSettings 与 ALevelScriptActor 体现了 Level 的管理职责。 | K2 |
| K4 | Level | Actor 排序与网络相关性决定了 Level 内部列表的组织方式。 | K2-K3 |
| K5 | Level | 关卡蓝图不鼓励复杂组件，是在控制认知复杂度。 | K3-K4 |
| K6 | World | World 是多个 Level 的拼装结果，不是单一关卡的别名。 | K1-K5 |
| K7 | World | PersistentLevel 是全局规则与初始加载的锚点。 | K6 |
| K8 | World | StreamingLevel 让加载/释放与玩法节奏解耦。 | K6-K7 |
| K9 | World | World 共享物理、导航等全局系统，但 Actor 仍按 Level 组织。 | K6-K8 |
| K10 | 总结 | UE 的世界模型是在“可管理性”与“动态加载”之间权衡的结果。 | K1-K9 |

### [AI注释-新增][K1] 为什么要有 Level 这一层

- 核心句：Level 的存在，是为了把巨大世界拆成可管理、可加载、可协作的局部单位。
- 为什么重要：如果所有对象都堆在一个 World 里，加载、协作与释放都会变得粗糙。
- 机制解释（L1-L2-L3）：
- L1 直觉层：把大地图切成多个区块，按需处理。
- L2 机制层：Level 让局部内容、局部规则、局部脚本可单独组织。
- L3 工程层：这为流式加载、团队并行编辑和局部性能优化提供了边界。
- 最小验证任务（10-15 分钟）：
- 任务：写 3 条“如果没有 Level，你会遇到什么管理问题”。
- 证据类型：风险清单。
- 过关标准：
- 至少 3 条。
- 每条都包含“问题 + 影响”。
- 本点结论：Level 是世界分治的第一层边界。
- 验点实验任务：提交清单并说明你认为最严重的一条问题。

### [AI注释-新增][K2] Level 不只是场景容器

- 核心句：Level 不只是摆放 Actor 的地方，它还承载了本关卡的脚本与规则。
- 为什么重要：这决定了 Level 在游戏逻辑中的位置，不只是编辑器里的文件。
- 机制解释（L1-L2-L3）：
- L1 直觉层：它像一块大陆，不只有地形，还有治理规则。
- L2 机制层：ALevelScriptActor 代表关卡脚本入口，允许关卡级事件处理。
- L3 工程层：关卡逻辑可以聚焦到某个区域，而不用扩散到整个 World。
- 最小验证任务（10-20 分钟）：
- 任务：写 5 条“Level 能做/不该做”的职责边界。
- 证据类型：职责表。
- 过关标准：
- 至少 5 条。
- 至少 2 条明确“不该做什么”。
- 本点结论：Level 是局部规则容器，不是全局逻辑容器。
- 验点实验任务：提交职责表并解释为什么不建议在 Level 写过多复杂逻辑。

### [AI注释-新增][K3] WorldSettings 与 ALevelScriptActor 的职责分工

- 核心句：WorldSettings 提供关卡级配置，ALevelScriptActor 提供关卡级行为。
- 为什么重要：二者一个偏数据，一个偏事件，是 Level 管理职责的两条线。
- 机制解释（L1-L2-L3）：
- L1 直觉层：一个管规则参数，一个管现场执行。
- L2 机制层：WorldSettings 常记录物理、光照、游戏模式等规则；LevelScriptActor 更像关卡脚本。
- L3 工程层：分开后更利于配置与行为解耦。
- 最小验证任务（10-20 分钟）：
- 任务：做一张“WorldSettings vs LevelScriptActor”对照表。
- 证据类型：对照表。
- 过关标准：
- 至少 4 条对照。
- 每条包含“数据/行为/可见性/作用域”中的至少两项。
- 本点结论：配置与行为分离能减少关卡维护混乱。
- 验点实验任务：提交对照表并说明你会把哪类逻辑放进 LevelScriptActor。

### [AI注释-新增][K4] Actor 排序为什么重要

- 核心句：Level 内部 Actors 的排序不仅是展示顺序，更是性能与网络组织的辅助结构。
- 为什么重要：排序会影响网络相关性判断和访问路径。
- 机制解释（L1-L2-L3）：
- L1 直觉层：把常用和非常用对象分区摆放，查找更快。
- L2 机制层：WorldSettings 固定靠前，网络相关 Actor 也会被分区处理。
- L3 工程层：这体现了 UE 对运行时访问模式的预先优化。
- 最小验证任务（15-20 分钟）：
- 任务：根据文中 SortActorList 逻辑，画一张排序流程图。
- 证据类型：流程图。
- 过关标准：
- 至少 4 个处理节点。
- 写出“非网络 Actor”和“网络 Actor”的分流规则。
- 本点结论：列表排序是运行时优化的一部分，不只是整理视图。
- 验点实验任务：提交流程图并说明 WorldSettings 为什么通常靠前。

### [AI注释-新增][K5] 为什么关卡蓝图不鼓励复杂组件

- 核心句：UE 通过隐藏关卡组件界面来减少 Level 层的认知负担。
- 为什么重要：这是一种用产品约束换取框架清晰度的设计。
- 机制解释（L1-L2-L3）：
- L1 直觉层：不是不给你工具，而是避免你把工具堆成杂物间。
- L2 机制层：Level 本质更像 Actor 容器，复杂行为应落到 Actor/Controller 层。
- L3 工程层：减少关卡层的复杂性，有利于可维护与团队协作。
- 最小验证任务（10-20 分钟）：
- 任务：列出 3 条“如果关卡允许随意加组件，可能出现的问题”。
- 证据类型：风险清单。
- 过关标准：
- 至少 3 条。
- 每条都给出具体后果。
- 本点结论：限制界面是为了降低误用，而不是功能不足。
- 验点实验任务：提交风险清单并说出你是否支持这种限制。

### [AI注释-新增][K6] World 不是一个更大的 Level

- 核心句：World 是多个 Level 的组织器与共享系统容器，不是单关卡的放大版。
- 为什么重要：这决定了你如何理解流式加载和跨关卡共享系统。
- 机制解释（L1-L2-L3）：
- L1 直觉层：World 像城市，Level 像街区。
- L2 机制层：World 统一管理多个 Level 的组合、加载与全局资源。
- L3 工程层：World 提供比 Level 更高层次的运行上下文。
- 最小验证任务（10-15 分钟）：
- 任务：写 4 条“World 与 Level 的区别”。
- 证据类型：对比表。
- 过关标准：
- 至少 4 条。
- 至少 1 条涉及“加载层级”。
- 本点结论：World 是组织层，Level 是内容层。
- 验点实验任务：提交对比表并用 30 秒复述二者关系。

### [AI注释-新增][K7] PersistentLevel 的锚点意义

- 核心句：PersistentLevel 是全局规则和初始加载的锚点，也是 World 的主配置来源。
- 为什么重要：它决定了哪个 Level 拥有“主导地位”。
- 机制解释（L1-L2-L3）：
- L1 直觉层：先有主城，再扩展周边区块。
- L2 机制层：WorldSettings 通常从 PersistentLevel 提取，作为世界级默认规则。
- L3 工程层：这样能保证初始进入世界时有稳定的基准配置。
- 最小验证任务（10-20 分钟）：
- 任务：写 3 条“为什么需要 PersistentLevel”的理由。
- 证据类型：理由清单。
- 过关标准：
- 至少 3 条。
- 至少 1 条涉及“默认规则来源”。
- 本点结论：PersistentLevel 是世界的基座。
- 验点实验任务：提交清单并说明你认为最关键的理由。

### [AI注释-新增][K8] StreamingLevel 的工程价值

- 核心句：StreamingLevel 让加载与释放可以分批发生，减少一次性压力。
- 为什么重要：这是开放世界和大地图体验可持续的关键。
- 机制解释（L1-L2-L3）：
- L1 直觉层：路过再开门，不是一次把所有门都打开。
- L2 机制层：流式加载把资源和对象生命周期拆开调度。
- L3 工程层：它直接影响卡顿、内存峰值和场景切换体验。
- 最小验证任务（15-20 分钟）：
- 任务：写一段“StreamingLevel 相比一次性全加载的 3 个收益”。
- 证据类型：收益说明。
- 过关标准：
- 至少 3 个收益。
- 每条都可观察（内存/帧率/体验）。
- 本点结论：流式加载是世界规模增长的必要技术。
- 验点实验任务：提交说明并给出你最关心的一个收益。

### [AI注释-新增][K9] World 共享系统但保留 Level 边界

- 核心句：World 统一物理、导航等系统，但 Actor 仍按 Level 组织，二者并不冲突。
- 为什么重要：这就是 UE 在“整体与局部”之间的平衡点。
- 机制解释（L1-L2-L3）：
- L1 直觉层：大家共用大路网，但各自住在不同小区。
- L2 机制层：World 负责共性系统，Level 保留局部归属。
- L3 工程层：这样既便于全局计算，又利于局部加载/卸载。
- 最小验证任务（10-20 分钟）：
- 任务：写 4 条“共享给 World 的系统”和 2 条“保留在 Level 的边界”。
- 证据类型：分类表。
- 过关标准：
- 至少 6 条。
- 要明确“为什么共享/为什么不共享”。
- 本点结论：共享系统不等于打散边界。
- 验点实验任务：提交分类表并说出你认为最典型的共享系统。

### [AI注释-新增][K10] 世界模型的权衡

- 核心句：UE 的世界组织是在“可管理性、协作性、加载效率”之间取平衡。
- 为什么重要：它解释了为什么 UE 选择分层而不是单一总容器。
- 机制解释（L1-L2-L3）：
- L1 直觉层：切块管理比一锅端更稳。
- L2 机制层：Level 边界便于协作，World 边界便于系统共享。
- L3 工程层：流式加载、多人协作、世界级规则都在这里达成平衡。
- 最小验证任务（10-15 分钟）：
- 任务：写 120 字总结，回答“如果没有 Level 边界，最先坏掉的会是什么”。
- 证据类型：复盘短文。
- 过关标准：
- 必须提到“加载/协作/边界”中的至少两项。
- 结论必须明确。
- 本点结论：世界模型是工程约束下的最优折中。
- 验点实验任务：提交短文并口述 30 秒总结。

### [AI注释-新增] 单篇文章通过规则（执行版）

1. 每个 K 点按 0-5 分评分，低于 3 分为未通过。
2. 未通过点必须补做最小验证任务并补交证据。
3. 全文通过条件：K 点通过率 >= 80%，且关键点 K2/K6/K8 均 >= 3 分。
4. 连续 2 个点未通过时，回退到上一个通过点重做 L1-L2 讲解。

### [AI注释-新增] 自测记录模板

| 日期 | 文章 | K点 | 得分(0-5) | 证据摘要 | 结论 | 下一步 |
| --- | --- | --- | --- | --- | --- | --- |
| YYYY-MM-DD | GamePlay架构（二）Level和World | K1 | 0-5 | 例如：World与Level对照表 | 通过/未通过 | 进入K2或补测 |

<!-- AI注释-新增结束 -->