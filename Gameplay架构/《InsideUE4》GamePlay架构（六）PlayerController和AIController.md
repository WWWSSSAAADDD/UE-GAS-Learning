---
title: "《InsideUE4》GamePlay架构（六）PlayerController和AIController"
source: "https://zhuanlan.zhihu.com/p/23649987"
author:
  - "[[大钊UE源码剖析，等编译的时候回答问题]]"
published:
created: 2026-04-18
description: "PlayerController：你不懂，伴君如伴虎啊 AIController：上来，我自己动引言上文我们谈到了Component-Actor-Pawn-Controller的结构，追溯了AController整个家族的崛起和身负的使命。本篇我们继续来探讨Controller…"
tags:
  - "clippings"
---
> PlayerController：你不懂，伴君如伴虎啊  
> AIController：上来，我自己动

## 引言

上文我们谈到了Component-Actor-Pawn-Controller的结构，追溯了AController整个家族的崛起和身负的使命。本篇我们继续来探讨Controller家族中最为人所知的PlayerController和AIController。  
作为一个Controller，我们讨论的依然是该如何控制。我们已经知道了Controller可以Possess并控制Pawn，但是Controller本身又是怎么驱动起来的呢？一个游戏里的控制角色大抵都可以分为两类：玩家和AI。不管是单机游戏或者分屏多玩家，还是网络玩家联机对战，游戏都是为了玩家服务的，所以也必然会有一个或多个玩家，就算是如《山》那种纯看的游戏，也是有一个“可观察不可动”的玩家的。而AI的实体的数量就可以是零或者多个。  
**Note1：** 依旧重申：输入、网络、AI行为树等模块虽跟PlayerController和AIController关系紧密，但目前都暂且不讨论，留待各自模块章节再叙述。

## APlayerController

让咱们先从简单的单机游戏开始讨论吧，比如一款单机FPS游戏，这个游戏里已经用各种各样的Actor们构建完成了世界场景，你的主角和敌人Pawn们也都在整装待发，这个时候你思考这么一个问题，我该怎么玩这个游戏？壮丽的舞台已经准备好了，就等你入场了。先抛开具体的引擎而言，首先你需要能看见（拥有Camera和位置），其次你必须能响应输入（玩家按WASD你应该能接收到），然后你可以根据输入操控一些Pawn（Possess然后传递Input），这样一个单机游戏中的简单玩家控制器就差不多了。一个游戏中只有一个PlayerController，在不同的关卡中你可以使用不同的PlayerController，但是同一时刻响应的只能是一个PlayerController。  
插上多个手柄，咱们再拓展一下，比如像《街霸》那种单PC但是多玩家对抗或者协作的游戏。两个玩家可以分别用两个手柄，或者一个用键盘一个用鼠标，甚至是键盘上的不同区域，形式可以多种多样。这个时候如果依然只有一个PlayerController，实现起来其实也是可行的，把两个手柄——所有的输入都由这个PlayerController来接收，然后在PlayerController内部再分别根据情况去处理不同的Pawn。但是这种方式的缺点显然也在于很容易把玩家1、2的输入和控制混杂在一起，没有清晰的区分开。因此，为了支持这种情况，我们可以开始允许游戏中同时出现多个PlayerController，每个PlayerController甚至都可以拥有自己的Viewport（分屏或者不同窗口），这样我们通过配置，可以精确的路由手柄1的输入给玩家1，各自的逻辑也很好的区分和复用。  
再插上网线继续，到了网游时代，我们的游戏就开始允许有多人联机对战了。玩家在自己的PC上控制的只是自己的本地的角色，而屏幕游戏里其他的玩家角色是由网线另一端的玩家控制的。为了更好的适应这种情况，我们就又得扩展一下PlayerController的概念，PlayerController不仅能控制本地的Pawn，而且还能“控制”远程的Pawn（实际上是通过Server上的PlayerController控制Server上的Pawn，然后再复制到远程机器上的Pawn实现的）。  
因此我们来看看UE里的PlayerController：  

![](https://pic3.zhimg.com/v2-88131e55febd8886e0f3c87b92c526e8_1440w.png)

PlayerController因为是直接跟玩家打交道的逻辑类，因此是UE里使用最多的类之一。UE4.13.2版本里1632行的.h文件和4686行的.cpp文件，里面实现了很多的功能，初阅读起来往往深陷其中不得要领。但是在上述的分析了之后，我们也可以在其中大概归纳出几个模块：

- Camera的管理，目的都是为了控制玩家的视角，所以有了PlayerCameraManager这一个关联很紧密的摄像机管理类，用来方便的切换摄像机。PlayerController的ControlRotation、ViewTarget等也都是为了更新Camera的位置。因为跟Camera的关系紧密，而Camera最后输出的是屏幕坐标里的图像，所以为了方便一些拾取的HitResult函数也都是实现在这里面。渲染章节会再详细介绍UE的摄像机管理。
- Input系统，包括构建InputStack用来路由输入事件，也包括了自己对输入事件的处理。所以包含了 [UPlayerInput](https://zhida.zhihu.com/search?content_id=1646070&content_type=Article&match_order=1&q=UPlayerInput&zhida_source=entity) 来委托处理。
- UPlayer关联，既然顾名思义是PlayerController，那自然要和Player对应起来，这也是PlayerController最核心的部分。一个UPlayer可以是本地的LocalPlayer，也可以是一个网络控制UNetConnection。PlayerController只有在SetPlayer之后，才可以开始正常工作。
- HUD显示，用于在当前控制器的摄像机面前一直显示一些UI，这是从UE3迁移过来的组件，现在用UMG的比较多，等介绍UI模块的时候再详细介绍。
- Level的切换，PlayerController作为网络里通道，在一起进行Level Travelling的时候，也都是先通过PlayerController来进行RPC调用，然后由PlayerController来转发到自己World中来实际进行。
- Voice，也是为了方便网络中语音聊天的一些控制函数。

简单来说，PlayerController作为玩家直接控制的实体，很多的跟玩家直接相关的操作也都得委托它来完成。目前来说PlayerController里旗下的100+的函数也大概可以分为以上几大模块，也根据需要重载了Controller里的一些其他函数。  
UE的思想是具象化一个“玩家实体”，并把所有的跟该玩家相关的操作和接口都交给它完成。一般其他的游戏引擎只是个“功能引擎”，提供了一些图形渲染UI系统等组件，但是在GamePlay这个层次就都非常欠缺了，一般都需要开发者自己搭建一套。而回想你写过的游戏，是不是也往往有一个Player类（一般是单件或者全局变量）？里面几乎是放着所有跟该玩家相关的业务逻辑代码。UE里的PlayerController就是这种概念，优点当然是直接方便好理解，缺点也如你所见，会代码膨胀得比较快。不过目前来说还算能接受，等某一块功能真的比较大了之后，可以再把它抽出一个单独的类来，如PlayerInput和PlayerCameraManager一样。

**思考：哪些逻辑应该放在PlayerController中？**  
回想我们上篇的问题：“哪些逻辑应该写在Controller中？”，该处的答案观点在本处也依然适用。不过我还想再补充几点：

- 对实现游戏逻辑来说，如果是按照MVC的视角，那么View对应的是Pawn的表现，而PlayerController对应的是Controller的部分，那Model就是游戏业务逻辑的数据了。拿超级马里奥游戏来举例子，把问题先局限在一个关卡内，假设要实现的是金币的逻辑，那么View指的是游戏右上角的金币数目UI，而玩家用PlayerController来控制马里奥来蹦跳行走，而马里奥（Pawn）通过触碰金币的事件又上报给PlayerController来相应增加金币。而PlayerController存储金币的数据就是在 [PlayerState](https://zhida.zhihu.com/search?content_id=1646070&content_type=Article&match_order=1&q=PlayerState&zhida_source=entity) 中。即PlayerState中有一个int coin，也有相应的AddCoin(int coin)。而PlayerController的职责应该是一边控制Pawn，一边负责内部正确的调用PlayerState的Coin接口。那么PlayerController里的成员变量有什么用？根据单一职责原则，我们写在哪个类里的变量应该尽量只符合该类的作用，所以PlayerController里的变量的意义在于更好的实现控制。比如假设玩家在一个关卡内可以按AABB来作弊获得100金币，但是限最多3次。那么这个按键的响应就应该由PlayerController来接收，然后调用AddCoin(100)，并更新PlayerController里的成员变量CoinCheatCount。也或者想实现马里奥的加速跑，也可以在PlayerController里增加Speed的成员变量。
- 记住PlayerController是可被替换的，不同的关卡里也可能是不一样的。比如马里奥在水下的时候控制的方式明显就不一样，所以就不能像“Player”单件类那样什么都往里面塞。这样一旦被替换掉了之后数据就都丢失了。
- PlayerController也不一定存在，考虑一下如果把马里奥做成联机游戏，那么对方玩家被同步过来的将只有PlayerState，对方玩家的PlayerController只在服务器上存在。所以这个时候，如果你把金币数据放在PlayerController里的话就非常尴尬了。所以为了扩展性来说，还是根据职责分明的原则来正确划分业务逻辑会比较好。
- 在任一刻，Player:PlayerController:PlayerState是1:1:1的关系。但是PlayerController可以有多个备选用来切换，PlayerState也可以相应多个切换。UPlayer的概念会在之后讲解，但目前可以简单理解为游戏里一个全局的玩家逻辑实体，而PlayerController代表的就是玩家的意志，PlayerState代表的是玩家的状态。

## AAIController

从某种程度上来说，AI也可以算是一个Player，只不过它不需要接收玩家的控制，可以自行决策行动。从玩家控制的逻辑需要有一个载体一样，AI的逻辑算法也需要有一个运行的实体。而这就是UE里的AIController：  

![](https://pic1.zhimg.com/v2-a0c2148ff8331da1b70ab4157e19f1c2_1440w.png)

同PlayerController对比，少了Camera、Input、UPlayer关联，HUD显示，Voice、Level切换接口，但也增加了一些AI需要的组件：

- Navigation，用于智能根据导航寻路，其中我们常用的MoveTo接口就是做这件事情的。而在移动的过程中，因为少了玩家控制的来转向，所以多了一个SetFocus来控制当前的Pawn视角朝向哪个位置。
- AI组件，运行启动行为树，使用黑板数据，探索周围环境，以后如果有别的AI算法方法实现成组件，也应该在本组件内组合启动。
- Task系统，让AI去完成一些任务，也是实现 [GameplayAbilities](https://zhida.zhihu.com/search?content_id=1646070&content_type=Article&match_order=1&q=GameplayAbilities&zhida_source=entity) 系统的一个接口。目前简单来说GameplayAbilities是为Actor添加额外能力属性集合的一个模块，比如HP，MP等。其中的GamePlayEffect也是用来实现Buffer的工具。另外GamePlayTags也是用来给Actor添加标签标记来表明状态的一种机制。目前来说该两个模块似乎都是由Epic的Game Team在维护，所以完成度不是非常的高，用的时候也往往需要根据自己情况去重构调整。

本文重点不在于讨论AI内部的各种组件功能，因此我们先把目光聚焦在AIController对象本身上。同PlayerController一样，AIController也只存在于Server上（单机游戏也可看作是Server）。游戏里必须有玩家参与，而AI可以没有，所以AIController并不一定会存在。我们可以在Pawn上配置AIControllerClass来让该Pawn产生的时候自动为它分配一个AIController，之后自动释放。

**思考：哪些逻辑应该放在AIController中？**  
我们依然要思考这个问题，大部分思想和原则和PlayerController是一样的，只不过AI算法的多种多样，所以我们推荐尽量利用UE提供的行为树黑板等组件实现，而不是直接在AIController硬编码再度实现。也请把目光仅仅局限在当前的Pawn身上，不要在里面写其他无关的逻辑。另外，因为AIController都是在关卡内比较短暂存在的，一般不太有跨Level的数据保存，所以你可以用AIController的成员变量来保存状态。而如果真的需要用到PlayerController的状态，则也可以引用一个PlayerState过来。如果想引用关卡的全局状态，也可以引用GameState，再更高级别的，甚至可以直接和GameInstance接触。  
但是AIController也可以通过配置bWantsPlayerState来获得自己的PlayerState，所以PlayerState其实也并不是跟UPlayer绑定的，毕竟从本质上来说APlayerState也只是个AInfo（AActor），跟其他Actor一样可以有多个，并没有什么稀奇的，区别是你自己怎么创建并利用它。

## 总结

到此，我们也算讨论完了Actor（Pawn）层次的控制，在这个层次上，我们关注的焦点在于如何更好的控制游戏世界里各种Actor交互和逻辑。UE采用了分化Actor的思维创建出AController来控制APawn们，因为玩家玩游戏也全都是控制着游戏里的一个化身来行动，所以UE抽象总结分化了一个APlayerController来上接Player的输入，下承Pawn的控制。对于那些自治的AI实体，UE给予了同样的尊重，创建出AIController，包含了一些方便的AI组件来实现游戏逻辑。并利用PlayerState来存储状态数据，支持在网络间同步。  

![](https://pica.zhimg.com/v2-3bd34e0947e07fe6b4e54b025977b3ac_1440w.png)

上图应该可以比较清晰的阐明，UE是如何充分利用Actor的本身机制来反过来实现对Actor的逻辑控制，相信亲爱的读者朋友们也能自行体会到它的优雅之处。对比其他的游戏引擎，往往它们都止步于Actor这一个层次，只提供了最基本的对象层次，美名其曰交给玩家控制。UE为我们提供了这一套简洁强大的机制，大大方便了我们编写逻辑的难度。

而下篇我们的逻辑之旅将再继续拔高一个层次，将开始讲解World层次的逻辑，这个世界的意志：GameMode！

上篇： [《InsideUE4》GamePlay架构（五）Controller](https://zhuanlan.zhihu.com/p/23480071)  
下篇： [《InsideUE4》GamePlay架构（七）GameMode和GameState](https://zhuanlan.zhihu.com/p/23707588)

## 引用

1. [PlayerController](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/latest/INT/Gameplay/Framework/Controller/PlayerController/index.html)
2. [AIController](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/latest/INT/Gameplay/Framework/Controller/AIController/index.html)

*UE 4.13.2*

\---------------------------------------------------------------------------------------------------------------------------

知乎专栏： [InsideUE4](https://zhuanlan.zhihu.com/insideue4)

UE4深入学习QQ群： **456247757** (非新手入门群，请先学习完官方文档和视频教程)

微信公众号： **aboutue** ，关于UE的一切新闻资讯、技巧问答、文章发布，欢迎关注。

**个人原创，未经授权，谢绝转载！**

2 人已送礼物

编辑于 2021-09-01 11:27[游戏引擎](https://www.zhihu.com/topic/19556258)[虚幻引擎](https://www.zhihu.com/topic/19824201)[游戏开发](https://www.zhihu.com/topic/19553361)