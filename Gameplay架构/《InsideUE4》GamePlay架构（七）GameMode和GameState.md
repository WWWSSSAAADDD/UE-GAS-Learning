---
title: "《InsideUE4》GamePlay架构（七）GameMode和GameState"
source: "https://zhuanlan.zhihu.com/p/23707588"
author:
  - "[[大钊UE源码剖析，等编译的时候回答问题]]"
published:
created: 2026-04-18
description: "我的世界，我做主引言上文我们说到在Actor层次，UE用Controller来充当APawn的逻辑控制者，也有了可以接受玩家输入的PlayerController，和能自行行动的AIController。Actor的逻辑编写介绍完了，那么本篇，我们继续…"
tags:
  - "clippings"
---
> 我的世界，我做主

## 引言

上文我们说到在Actor层次，UE用Controller来充当APawn的逻辑控制者，也有了可以接受玩家输入的PlayerController，和能自行行动的AIController。Actor的逻辑编写介绍完了，那么本篇，我们继续爬升，对于由Actors组成的Level这一层次，UE又是怎么控制的呢？  
对Level记不太清楚的朋友，可以翻回去查看“GamePlay架构（二）Level和World”的讲述，简单概括就是World是由一个 [PersistentLevel](https://zhida.zhihu.com/search?content_id=1669170&content_type=Article&match_order=1&q=PersistentLevel&zhida_source=entity) 和一些 [subLevels](https://zhida.zhihu.com/search?content_id=1669170&content_type=Article&match_order=1&q=subLevels&zhida_source=entity) 组成的，PersistentLevel切换了，相应的World也会切换。所以本文的关注点是在这么一个对象层次结构下，UE是怎么设计的，我们又能做些什么。

## GameMode

Level，在游戏里的概念里，就是关卡的意思。同时作为游戏的玩家和开发者，我们总是会非常经常的提起关卡，但是关卡具体又是个什么定义呢？游戏里的哪些部分可以算是一个关卡？简单的我们都知道有《愤怒的小鸟》或《植物大战僵尸》的关卡，复杂的有大型FPS游戏里的关卡，而对于更大型的《暗黑3》或者大型无缝地图RPG游戏《巫师3》，甚至是号称超级广阔宇宙《无人深空》，我们能直接了当的说出哪部分是关卡吗？游戏行业发展如今，为了更好的组织游戏逻辑和内容资源，也发展出了一些概念来更好的理解和阐述，虽然叫法不同，不过含义理念都是相通的。比如，Cocos2dx会认为游戏就是由不同的Scene切换组成的，每个Scene又由Layer组成；Unity也认为游戏就是一个个Scene；而UE的视角的是，游戏是由一个个World组成的，World又是由Level组成的。这些概念有什么不同？  
让我们从游戏本身的机制上分析：

- 游戏或玩家的节奏，游戏可以分成一个个阶段，马里奥里的关卡就是一个阶段，而RPG游戏的一个大地图也是一个阶段。一个游戏也可能只有一个阶段，比如一直在宇宙里漫游的游戏。通常一个阶段结束后，会有一个结算，阶段之间，玩家也能明显感觉到切换感。
- 游戏的机制，有时候即使是同样的场景，玩家却也能感觉就像在玩两个不同的游戏，比如MOBA里的同一张地图上的各种不同挑战模式。
- 游戏的资源划分，有时候也能遇见同一个玩法应用在不同的场景上，比如赛车游戏的不同跑道。有时候也会在游戏的大地图里从酷热的沙漠到寒冷的极地。游戏开发中也总是倾向于给游戏用到的资源划分成组的进行载入和释放。

通过以上的分析，也和以前的一贯思路一样，我们发现在思考“关卡”这件事情上，也是要保持头脑清晰的分清“表示”和“逻辑”。玩法就是“逻辑”，场景就是“表示”。所以我们如果以逻辑来划分游戏，得到的就是一个个World的概念；如果以表示来划分，得到就是一个个Level。一场游戏中，玩法再复杂但也只有一个，场景却可以无限大，所以可以有很多个表示拼接组装，因此是World包含Level，而不是反过来。现在回过头来回想一下Cocos2dx和Unity的世界观，它们的概念还只是在表示层，在游戏实例和关卡之间少了一个更高级的逻辑概念。  
因此UE的世界观是，World更多是逻辑的概念，而Level是资源场景表示。以《巫师3》为例，有好几个国家之间通过传送切换，国家内大地图无缝漫游，显然我们知道不可能把一个国家的所有资源都加载进内存，因此在UE里，一个国家就是许多个Level拼接的，而一个国家就是一个World，它们可以有不同的模式玩法。但毕竟AAA游戏很少，通常的，我们的游戏比较简单的用一个Level就够了，否则这个场景表示的概念就应该叫Area更合适了，也因此通常的这里的Level也常常对应游戏里玩家面对的"关卡"，也因此UE里Level的Settings叫做 [WorldSettings](https://zhida.zhihu.com/search?content_id=1669170&content_type=Article&match_order=1&q=WorldSettings&zhida_source=entity) 了。  
厘清了这些概念了之后，我们就知道，当我们在谈Level的业务逻辑控制的时候，我们实际上谈的是World的业务逻辑。按照UE的设计理念和经过Controller的经历，我想我也不用多解释了从Actor再派生出一个WorldController的方式了，可以直接的享受Actor已经提供的一切福利。一个World的Controller想不出有什么需要展示渲染的，因此可以直接从AInfo派生吧。哦，WorldController是我瞎编的，在UE3里它叫做 [GameInfo](https://zhida.zhihu.com/search?content_id=1669170&content_type=Article&match_order=1&q=GameInfo&zhida_source=entity) ，到了UE4它改名为了GameMode。笼统的讲，一个World就是一个Game，把玩法叫做Mode，我们应该也能接受吧。那我们来看看它：  

![](https://pic4.zhimg.com/v2-df7c0a12f2b806dbb60e84a95ae9758d_1440w.png)

既然勇敢的承担了游戏逻辑的职责，说他是AInfo家族里的扛把子也不为过，因此GameMode身为一场游戏的唯一逻辑操纵者身兼重任，在功能实现上有许多的接口，但主要可以分为以下几大块：

1. **Class登记** ，GameMode里登记了游戏里基本需要的类型信息，在需要的时候通过UClass的反射可以自动Spawn出相应的对象来添加进关卡中。前文说过的Controller的类型登记也是在此，GameMode就是比Controller更高一级的领导。
![](https://pic1.zhimg.com/v2-42d75bd239bcad7995349a712f1ba37e_1440w.png)

1. **游戏内实体的Spawn** ，不光登记，GameMode既然作为一场游戏的主要负责人，那么游戏的加载释放过程中涉及到的实体的产生，包括玩家Pawn和PlayerController，AIController也都是由GameMode负责。最主要的SpawnDefaultPawnFor、SpawnPlayerController、ShouldSpawnAtStartSpot这一系列函数都是在接管玩家实体的生成和释放，玩家进入该游戏的过程叫做Login（和服务器统一），也控制进来后在什么位置，等等这些实体管理的工作。GameMode也控制着本场游戏支持的玩家、旁观者和AI实体的数目。
2. **游戏的进度** ，一个游戏支不支持暂停，怎么重启等这些涉及到游戏内状态的操作也都是GameMode的工作之一，SetPause、ResartPlayer等函数可以控制相应逻辑。
3. **Level的切换** ，或者说World的切换更加合适，GameMode也决定了刚进入一场游戏的时候是否应该开始播放开场动画（cinematic），也决定了当要切换到下一个关卡时是否要bUseSeamlessTravel，一旦开启后，你可以重载GameMode和PlayerController的GetSeamlessTravelActorList方法和GetSeamlessTravelActorList来指定哪些Actors不被释放而进入下一个World的Level。
4. **多人游戏的步调同步** ，在多人游戏的时候，我们常常需要等所有加入的玩家连上之后，载入地图完毕后才能一起开始逻辑。因此UE提供了一个 [MatchState](https://zhida.zhihu.com/search?content_id=1669170&content_type=Article&match_order=1&q=MatchState&zhida_source=entity) 来指定一场游戏运行的状态，意义看名称也是不言自明的，就是用了一个状态机来标记开始和结束的状态，并触发各种回调。  
	/\*\* Possible state of the current match, where a match is all the gameplay that happens on a single map \*/  
	namespace MatchState  
	{  
	extern ENGINE\_API const FName EnteringMap; // We are entering this map, actors are not yet ticking  
	extern ENGINE\_API const FName WaitingToStart; // Actors are ticking, but the match has not yet started  
	extern ENGINE\_API const FName InProgress; // Normal gameplay is occurring. Specific games will have their own state machine inside this state  
	extern ENGINE\_API const FName WaitingPostMatch; // Match has ended so we aren't accepting new players, but actors are still ticking  
	extern ENGINE\_API const FName LeavingMap; // We are transitioning out of the map to another location  
	extern ENGINE\_API const FName Aborted; // Match has failed due to network issues or other problems, cannot continue  
	}

**思考：多个Level配置不同的GameMode时采用的是哪一个GameMode？**  
我们知道除了配置全局的GameModeClass之外，我们还能为每个Level单独的配置不同的GameModeClass。但是当一个World由多个Level组成的时候，这样就相当于配置了多个GameModeClass，那么应用的是哪一个？首先第一个原则需要记住的就是，一个World里只会有一个GameMode实例，否则肯定乱套了。因此当有多个Level的时候，一定是PersistentLevel和多个StreamingLevel，这时就算它们配置了不同的GameModeClass，UE也只会为第一次创建World时加载PersistentLevel的时候创建GameMode，在后续的LoadStreamingLevel时候，并不会再动态创建出别的GameMode，所以GameMode从始至终只有一个，PersistentLevel的那个。

**思考：Level迁移时GameMode是否保持一致？**  
在在travelling的时候，如果下一个Level的配置的GameModeClass和当前的不同，那么迁移后是哪个GameMode？  
无论travelling采用哪种方式，当前的World都会被释放掉，然后加载创建新的World。但这个过程中，有点区别的是根据bUseSeamlessTravel的不同，UE可以选择哪些Actor迁移到下一个World中去（实现方式是先创建个中间过渡World进行二段迁移（为了避免同时加载进两个大地图撑爆内存），具体见引用3）。分两种情况：  
不开启bUseSeamlessTravel，那么在travelling的时候（ServerTravel或ClientTravel），当前的World会被释放，所以当前的GameMode就被释放掉。新的World加载，就会根据新的GameModeClass创建新的GameMode。所以这时是不同的。  
开启bUseSeamlessTravel，travelling时，当前World的GameMode会调用GetSeamlessTravelActorList：

```cpp
void AGameMode::GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList)
{
    UWorld* World = GetWorld();

    // Get allocations for the elements we're going to add handled in one go
    const int32 ActorsToAddCount = World->GameState->PlayerArray.Num() + (bToTransition ?  3 : 0);
    ActorList.Reserve(ActorsToAddCount);

    // always keep PlayerStates, so that after we restart we can keep players on the same team, etc
    ActorList.Append(World->GameState->PlayerArray);

    if (bToTransition)
    {
        // keep ourselves until we transition to the final destination
        ActorList.Add(this);
        // keep general game state until we transition to the final destination
        ActorList.Add(World->GameState);
        // keep the game session state until we transition to the final destination
        ActorList.Add(GameSession);

        // If adding in this section best to increase the literal above for the ActorsToAddCount
    }
}
```

在第一步从CurrentWorld到TransitionWorld的迁移时候，bToTransition==true，这个时候GameMode也会迁移进TransitionWorld（TransitionMap可以在ProjectSettings里配置），也包括GameState和GameSession，然后CurrentWorld释放掉。第二步从TransitionWorld到NewWorld的迁移，GameMode（已经在TransitionWorld中了）会再次调用GetSeamlessTravelActorList，这个时候bToTransition==false，所以第二次的时候如代码所见当前的GameMode、GameState和GameSession就被排除在外了。这样NewWorld再继续InitWorld的时候，一发现当前没有GameMode，就会根据配置的GameModeClass重新生成一个出来。所以这个时候GameMode也是不同的。  
结论是，UE的流程travelling，GameMode在新的World里是会新生成一个的，即使Class类型一致，即使bUseSeamlessTravel，因此在travelling的时候要小心GameMode里保存的状态丢失。不过Pawn和Controller默认是一致的。

**思考：哪些逻辑应该写在GameMode里？哪些应该写在Level Blueprint里？**  
我们依旧要问这个老土的问题。根据我们前面的知识，我们知道每个Level其实也是有自己的LevelScriptActor的，那么这两个有什么区别？可以从这几个方面来回答：

- 概念上，Level是表示，World是逻辑，一个World如果有很多个Level拼在一起，那么也就是有了很多个LevelScriptActor，无法想象在那么多个地方写一个完整的游戏逻辑。所以GameMode应该专注于逻辑的实现，而LevelScriptActor应该专注于本Level的表示逻辑，比如改变Level内某些Actor的运动轨迹，或者某一个区域的重力，或者触发一段特效或动画。而GameMode应该专注于玩法，比如胜利条件，怪物刷新等。
- 组合上，同Controller应用到Pawn一样道理，因为GameMode是可以应用在不同的Level的，所以通用的玩法应该放在GameMode里。
- GameMode只在Server存在（单机游戏也是Server），对于已经连接上Server的Client来说，因为游戏的状态都是由Sever决定的，Client只是负责展示，所以Client上是没有GameMode的，但是有LevelScriptActor，所以GameMode里不要写Client特定相关的逻辑，比如操作UI等。但是LevelScriptActor还是有的，而且支持RPC，即使如此，LevelScriptActor还是应该只专注于表现，比如网络中触发一个特效火焰。至于UI，可以通过PlayerController的RPC，然后转发到GameInstance来操作。
- 跟下层的PlayerController比较，GameMode关心的是构建一个游戏本身的玩法，PlayerController关心的玩家的行为。这两个行为是独立正交可以自由组合的。所以想想哪些逻辑属于游戏，哪些属于玩家，就应该清楚写在哪里了。
- 跟上层的GameInstance比较，GameInstance关注的是更高层的不同World之间的逻辑，虽然有时候他也把手伸下来做些UI的管理工作，不过严谨来说，在UE里UI是独立于World的一个结构，所以也还算能理解。因此可以把不同GameMode之间协调的工作交给GameInstance，而GameMode只专注自己的玩法世界。

## GameState

上回说到了APlayerState用来保存玩家的游戏数据，那么同样的，对于一场游戏，也需要一个State来保存当前游戏的状态数据，比如任务数据等。跟APlayerState一样，GameState也选择从AInfo里继承，这样在网络环境里也可以Replicated到多个Client上面去。  

![](https://picx.zhimg.com/v2-336fd667fb674bf176fa198741eec129_1440w.png)

比较简单，第一个MatchState和相关的回调就是为了在网络中传播同步游戏的状态使用的（记得GameMode在Client并不存在，但是GameState是存在的，所以可以通过它来复制），第二部分是玩家状态列表，同样的如果在Client1想看到Client2的游戏状态数据，则Client2的PlayerState就必须广播过来，因此GameState把当前Server的PlayerState都收集了过来，方便访问使用。  
关于使用，开发者可以自定义GameState子类来存储本GameMode的运行过程中产生的数据（那些想要replicated的!），如果是GameMode游戏运行的一些数据，又不想要所有的客户端都可以看到，则也可以写在GameMode的成员变量中。重复遍，PlayerState是玩家自己的游戏数据，GameInstance里是程序运行的全局数据。

### GameSession

是在网络联机游戏中针对Session使用的一个方便的管理类，并不存储数据，本文重点也不在网络，故不做过多解释，可暂时忽略，留待网络章节再讨论。在单机游戏中，也存在该类对象用来LoginPlayer，不过因为只是作为辅助类，那也可看作GameMode本身的功能，所以不做过多讨论。

## 总结

现在，我们也算讨论完了Level（World）层次的控制，对于一场游戏而言，我们最关心的是怎么协调好整个场景的表现（LevelBlueprint）和游戏玩法的编写（GameMode）。UE再次用Actor分化派生的思想，用同样套路的AGameMode和AGameState支持了玩法和表现的解耦分离和自由组合，并很好的支持了网络间状态的同步。同时也提供了一个逻辑的实体来负责创建关系内那些关键的Pawn和Controller们，在关卡切换（World）的时候，也有了一个负责对象来处理一些本游戏的特定情况处理。  

![](https://pic1.zhimg.com/v2-732b99e7784299a93449256f076eea76_1440w.png)

我们的逻辑之旅还没到终点，让我们继续爬升，下篇将介绍Player。

上篇： [《InsideUE4》GamePlay架构（六）PlayerController和AIController](https://zhuanlan.zhihu.com/p/23649987)

下篇： [《InsideUE4》GamePlay架构（八）Player](https://zhuanlan.zhihu.com/p/23826859)

## 修订

在笔者书写本篇的同时（UE4.13.2），UE同时也完成了4.14的preview3的工作，roadmap里“GameMode Cleanup”的工作也已经完成了，第二天发现4.14正式发布了。因此为了紧跟UE最新潮流时尚，以后要是文章内容所涉及内容被UE修改完善优化的，也会采用修订的方式进行补充说明，之后不再特意作此声明。

### 4.14 GameMode，GameState的清理

根据搜索到的最早记录" [\[Request/Improvment\] GameMode cleanup.](https://link.zhihu.com/?target=https%3A//forums.unrealengine.com/showthread.php%3F39840-Request-Improvment-GameMode-cleanup)"(09-14-2014)，是有人抱怨当前的GameMode实现了太多的默认逻辑（例如多人的Match），虽然方便了一些人使用，但是也确实加大了理解的难度，并且有时候还得去屏蔽删除一些默认逻辑。然后顺便吐槽了一番AActor里的Damage，笔者也表示这确实不是AActor应该管的事情。  
言归正传，UE在2016-08-24的时候开始加进roadmap，并终于在4.14里实现完成了。如前所述，就是把GameMode和GameState的一些共同最基础部分抽到基类AGameModeBase和AGameStateBase里，并把现在的GameMode和GameState依然当作多人联机的默认实现。所以以后大家如果想实现一个比较简单的单机GameMode就可以直接从AGameModeBase里继承了。

![](https://pic3.zhimg.com/v2-77a448b9caf4758f1b45cbee9bddb1e8_1440w.png)

可以看到，其实就是把MatchState给往下拉了一层，并把一些多玩家控制的逻辑，合起来就是网络联机游戏的默认逻辑给抽离开了。同样的对于GameState也做了处理：  

![](https://pic1.zhimg.com/v2-7d2b6410a98ae6819a358e59b5cf8eaa_1440w.png)

把MatchState也抽离到了下层，并增加了几个方便的字段引用（如AuthorityGameMode）。总体功能职责架构上还是没有什么大变化的，吓死我了。

## 引用

1. [GameMode](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/latest/INT/Gameplay/Framework/GameMode/index.html)
2. [GameState](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/latest/INT/Gameplay/Framework/GameState/index.html)
3. [Travelling in Multiplayer](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/latest/INT/Gameplay/Networking/Travelling/index.html)

*UE4.13.2*

\---------------------------------------------------------------------------------------------------------------------------

知乎专栏： [InsideUE4](https://zhuanlan.zhihu.com/insideue4)

UE4深入学习QQ群： **456247757** (非新手入门群，请先学习完官方文档和视频教程)

微信公众号： **aboutue** ，关于UE的一切新闻资讯、技巧问答、文章发布，欢迎关注。

**个人原创，未经授权，谢绝转载！**

2 人已送礼物

编辑于 2021-09-01 11:36[游戏引擎](https://www.zhihu.com/topic/19556258)[虚幻引擎](https://www.zhihu.com/topic/19824201)[游戏开发](https://www.zhihu.com/topic/19553361)

---

<!-- AI注释-新增开始：以下内容为教学增补，不属于原始文章正文 -->
## [AI注释-新增] 三步学习法拆解与验点（K1-K10）

> [AI注释说明]
> 1. 本区为 AI 新增教学内容，原文正文未做任何删改。
> 2. 每次只推进 1 个 K 点，当前点未通过验证前不得进入下一点。
> 3. 每个 K 点都包含：核心句、为什么重要、机制解释、思考题、考察重点、参考答案。

### [AI注释-新增] K点总览

| 编号 | 对应原文章节 | 核心句 | 前置条件 |
| --- | --- | --- | --- |
| K1 | 引言 | GameMode 和 GameState 是 UE 在 World 层面处理“玩法与状态”的核心对象。 | 无 |
| K2 | GameMode | World 的划分决定了 GameMode 的作用范围。 | K1 |
| K3 | GameMode | GameMode 更偏“玩法规则”和“世界管理”，而不是纯展示。 | K1-K2 |
| K4 | GameMode | GameMode 负责类登记、实体 Spawn、进度、切换和多人步调。 | K3 |
| K5 | GameMode | GameMode 的默认生命周期只在当前 World 中有效。 | K2-K4 |
| K6 | GameMode | Level Blueprint 和 GameMode 关注点不同：前者偏局部表现，后者偏全局玩法。 | K3-K5 |
| K7 | GameState | GameState 负责把当前游戏状态同步给客户端。 | K1-K6 |
| K8 | GameState | PlayerState 列表和 MatchState 是 GameState 的核心数据。 | K7 |
| K9 | GameSession | GameSession 是网络会话辅助对象，不是玩法核心。 | K7-K8 |
| K10 | 总结 | GameMode+GameState 共同完成“玩法规则 + 状态同步”的双层职责。 | K1-K9 |

### [AI注释-新增][K1] 本篇在整体架构中的位置

- 核心句：这一篇开始，UE 在 World 层面继续补齐“玩法控制”和“状态同步”的组织方式。
- 为什么重要：如果只懂 Actor/Pawn/Controller，不懂 World 层的玩法控制，就看不懂一场游戏是怎么被整体管理的。
- 机制解释：
- L1 直觉层：棋子会动只是开始，真正要决定的是整盘棋的规则。
- L2 机制层：World 需要一个负责玩法的对象，也需要一个负责状态的对象。
- L3 工程层：GameMode 和 GameState 正是用来承接这两件事的。
- 思考题：为什么说本篇讨论的是“玩法层”，而不是单个角色层？
- 考察重点：是否能意识到本篇已经从 Pawn/Controller 上升到 World 级管理。
- 参考答案：因为这里讨论的不再是某个角色怎么动，而是整个 World 的规则、进度、同步和状态如何组织，这已经超出了单个 Pawn 或 Controller 的范围。

### [AI注释-新增][K2] World 决定 GameMode 的作用范围

- 核心句：GameMode 的职责范围不是单个 Level，而是当前 World。
- 为什么重要：这能帮助你区分关卡表示和玩法规则的边界。
- 机制解释：
- L1 直觉层：一个国家有一套政体，不是每条街都换一套政府。
- L2 机制层：PersistentLevel 切换会导致 World 切换，而 GameMode 绑定的是 World 的玩法。
- L3 工程层：因此同一个 GameMode 不是“挂在某个小关卡上”，而是服务当前 World。
- 思考题：为什么一个 World 只应该有一个 GameMode？
- 考察重点：是否能联系“玩法统一性”和“状态管理清晰度”。
- 参考答案：因为 GameMode 表达的是整个世界的玩法规则和控制逻辑，如果同一个 World 里同时存在多个 GameMode，规则来源就会冲突，实体生成、状态切换和同步逻辑都会混乱，所以 UE 只允许一个 World 对应一个 GameMode 实例。

### [AI注释-新增][K3] GameMode 的定位

- 核心句：GameMode 是一场游戏的“玩法负责人”，不是展示负责人。
- 为什么重要：这能让你知道哪些逻辑应该放进 GameMode，哪些不应该。
- 机制解释：
- L1 直觉层：它像裁判和总导演，管规则怎么跑。
- L2 机制层：GameMode 不直接负责画面展示，而是控制游戏规则、实体生成和流程节点。
- L3 工程层：它的职责更像“游戏本身的执行层”而不是“某个角色的细节层”。
- 思考题：为什么说 GameMode 更适合承载“玩法”，而不适合承载“表现”？
- 考察重点：是否能分清“世界规则”和“局部视觉表现”。
- 参考答案：因为玩法是全局规则，需要统一控制开始、结束、胜利、失败、出生点和玩家接入等；而表现往往是局部场景或角色层面的事情，把展示也塞进 GameMode 会让它职责过重。

### [AI注释-新增][K4] GameMode 主要在做什么

- 核心句：GameMode 负责类登记、实体 Spawn、进度控制、关卡切换和多人步调同步。
- 为什么重要：这五块基本覆盖了“一个游戏怎么启动、怎么跑、怎么结束”。
- 机制解释：
- L1 直觉层：先发牌、再开局、再管理节奏、再切场景。
- L2 机制层：它会决定用什么 Controller、Pawn、PlayerState 等类，并负责生成和接管。
- L3 工程层：它还会影响暂停、重启、seamless travel 和 match 流程。
- 思考题：为什么类登记和实体 Spawn 也应该算在 GameMode 的职责里？
- 考察重点：是否能理解“规则决定谁应该被创建”。
- 参考答案：因为玩法规则会直接影响要生成哪些实体、用哪种 Controller、玩家出生在哪里以及是否允许某些对象迁移；类登记和 Spawn 本质上是把玩法规则落实成实际对象。

### [AI注释-新增][K5] 为什么 GameMode 只在当前 World 有效

- 核心句：GameMode 的生命周期跟随 World，World 被替换时它也会被替换。
- 为什么重要：这决定了哪些状态可以放进 GameMode，哪些不可以。
- 机制解释：
- L1 直觉层：一张地图换了，上一张地图的管理者自然也要换。
- L2 机制层：OpenLevel 或 World 切换会销毁当前 World，并基于新配置创建新的 GameMode。
- L3 工程层：所以 GameMode 里不适合放跨 World 的长期状态。
- 思考题：如果把跨关卡数据放在 GameMode 里，会发生什么？
- 考察重点：是否能把“World 生命周期”和“GameMode 生命周期”对应起来。
- 参考答案：会在切换 World 时丢失，因为 GameMode 会随着当前 World 一起销毁和重建；跨关卡状态应该放到 GameInstance 或其他更长生命周期的对象里。

### [AI注释-新增][K6] GameMode 与 Level Blueprint 的区别

- 核心句：GameMode 偏玩法全局，Level Blueprint 偏当前关卡表现和局部事件。
- 为什么重要：这是很多项目里最容易写乱的地方。
- 机制解释：
- L1 直觉层：一个管总规则，一个管某个房间里的小机关。
- L2 机制层：GameMode 只在 Server 侧存在，Level Blueprint 则更适合局部事件和表现逻辑。
- L3 工程层：如果把 UI 或客户端特定逻辑写进 GameMode，就会出问题。
- 思考题：哪些逻辑应该放在 GameMode，哪些更适合放在 Level Blueprint？
- 考察重点：是否能区分“玩法规则”和“场景表现”。
- 参考答案：胜负条件、玩家生成、比赛流程、全局玩法规则适合 GameMode；某个关卡内的机关动画、局部特效、区域触发等更适合 Level Blueprint。UI 之类客户端表现通常也不应放进 GameMode。

### [AI注释-新增][K7] GameState 的角色

- 核心句：GameState 负责保存并同步“当前这一局游戏”的状态。
- 为什么重要：它是客户端看到游戏状态的主要来源之一。
- 机制解释：
- L1 直觉层：GameMode 是裁判，GameState 是记分牌。
- L2 机制层：GameState 会把游戏状态复制给多个客户端，让大家看到一致的局面。
- L3 工程层：GameMode 不在客户端存在，但 GameState 在，因此它承担了同步桥梁的作用。
- 思考题：为什么 GameState 要和 GameMode 分开，而不是把状态全放到 GameMode？
- 考察重点：是否能说出“客户端可见”和“服务器权威”这两个维度。
- 参考答案：因为 GameMode 只在服务器端存在，不适合直接给客户端看；GameState 则可以复制到客户端，用来承载需要同步给所有人的游戏状态，所以二者分工不同。

### [AI注释-新增][K8] MatchState 与 PlayerArray 的意义

- 核心句：MatchState 用来描述一局游戏的阶段，PlayerArray 用来收集当前玩家状态。
- 为什么重要：它们是多人游戏同步和流程推进的核心数据。
- 机制解释：
- L1 直觉层：比赛有开局、进行中、结束等阶段。
- L2 机制层：MatchState 通过状态机表达当前比赛所处阶段，PlayerArray 让客户端能看到各个玩家的状态。
- L3 工程层：这为多人联机中的开始同步、结算和阶段切换提供了统一机制。
- 思考题：为什么多人游戏需要 MatchState，而不是简单一个布尔值表示“开始/结束”？
- 考察重点：是否能理解状态机比二元开关更适合复杂流程。
- 参考答案：因为比赛流程通常不止“开始/结束”两种状态，还可能有进入地图、等待开始、进行中、结束后等待、离开地图、异常中止等阶段；MatchState 用一组状态来表达更复杂的流程，能让回调和逻辑切换更清晰。

### [AI注释-新增][K9] GameSession 的位置

- 核心句：GameSession 是网络会话辅助对象，不是玩法本体。
- 为什么重要：它帮助你把“玩法逻辑”和“联机会话管理”分开。
- 机制解释：
- L1 直觉层：它像会场接待员，负责登记但不决定比赛规则。
- L2 机制层：GameSession 主要服务于联机会话、登录和接入管理。
- L3 工程层：它更像 GameMode 的辅助，而不是游戏核心规则承载者。
- 思考题：为什么 GameSession 不适合承担主要玩法逻辑？
- 考察重点：是否能区分“会话辅助”和“游戏规则”。
- 参考答案：因为 GameSession 更关注玩家接入、会话管理等网络辅助职责，不是决定胜负、刷新怪物或推进比赛流程的核心对象；把玩法逻辑放进它会让会话管理与游戏规则混在一起。

### [AI注释-新增][K10] 本篇的最终结论

- 核心句：GameMode 和 GameState 共同完成“玩法规则 + 状态同步”的双层职责。
- 为什么重要：这是 UE 在 World 层面组织游戏的关键结构。
- 机制解释：
- L1 直觉层：一个管规则，一个管记分和广播。
- L2 机制层：GameMode 负责世界玩法和实体生成，GameState 负责把当前游戏状态同步给客户端。
- L3 工程层：二者配合后，UE 才能把“场景表示”和“游戏规则”分清。
- 思考题：如果没有 GameState，客户端如何知道当前比赛的状态？如果没有 GameMode，谁来决定玩法？
- 考察重点：是否能把 GameMode、GameState、PlayerState、GameInstance 的职责区分清楚。
- 参考答案：没有 GameState，客户端就很难可靠地看到服务器上的当前比赛状态和玩家列表；没有 GameMode，就没有一个权威的对象来决定玩法、生成实体和推进流程。二者各管一半，分别对应规则与状态。

### [AI注释-新增] 单篇文章通过规则（执行版）

1. 每个 K 点按 0-5 分评分，低于 3 分视为未通过。
2. 未通过点必须补做验证任务并补交证据。
3. 全文通过条件：K 点通过率 >= 80%，且关键点 K3/K4/K7/K8 均 >= 3 分。
4. 若连续 2 个点未通过，回退到上一个通过点重做 L1-L2 讲解。

### [AI注释-新增] 自测记录模板

| 日期 | 文章 | K点 | 得分(0-5) | 证据摘要 | 结论 | 下一步 |
| --- | --- | --- | --- | --- | --- | --- |
| YYYY-MM-DD | GamePlay架构（七）GameMode和GameState | K1 | 0-5 | 例如：GameMode / GameState 对照表 | 通过/未通过 | 进入K2或补测 |

<!-- AI注释-新增结束 -->