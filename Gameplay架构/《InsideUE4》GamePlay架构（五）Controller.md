---
title: "《InsideUE4》GamePlay架构（五）Controller"
source: "https://zhuanlan.zhihu.com/p/23480071"
author:
  - "[[大钊UE源码剖析，等编译的时候回答问题]]"
published:
created: 2026-04-18
description: "那一天 Pawn又回想起了 被Controller所支配的恐惧引言如上文所述，UE从Actor中分化了一些专门可供玩家“控制”的Pawn，那我们这篇就专门来谈谈该怎么个控制法！ 所谓的控制，本质指的就是我们游戏的业务逻辑。比如…"
tags:
  - "clippings"
---
> 那一天  
> Pawn又回想起了  
> 被Controller所支配的恐惧

## 引言

如上文所述，UE从Actor中分化了一些专门可供玩家“控制”的Pawn，那我们这篇就专门来谈谈该怎么个控制法！  
所谓的控制，本质指的就是我们游戏的业务逻辑。比如说玩家按A键，角色自动找一个最近的敌人并攻击，这个自动寻找目标并攻击的逻辑过程，就是我们所谈的控制。  
Note1：重申一下，Controller特别是PlayerController，跟网络，AI和Input的关系都非常的紧密，目前都暂且不讨论，留待各自模块章节再叙述。

## MVC

不管是游戏，还是其他App，Web或Server等，本质上都是程序，所以也都是或多或少需要一些程序逻辑。从1843年拜伦的女儿Ada Lovelace用穿孔机编写第一个程序开始，到2016的今天我们能方便地用蓝图连线组织程序逻辑，应该归功于一代代软件工程师们孜孜不倦的探索。时代在发展，技术在进步，软件也愈趋于复杂多变，很多软件的庞大也已经超越了个人的理解容量极限（UE？），因此我们就越来越需要设计方法来让我们可管理庞大的复杂度。几十年的迭代，旧的模型被放弃，新的模型被提出验证，工程师们在这过程中总结积累出了一些设计模式。最负有盛名的应该是 [GOF](https://zhida.zhihu.com/search?content_id=1578067&content_type=Article&match_order=1&q=GOF&zhida_source=entity) 的《设计模式》，以及MVC，MVP，MVVM等。本文的重点不在于细谈论各种设计模式，如果有对设计模式不清楚的读者，请务必仔细去研究学习，因UE如此庞大的代码框架也是充斥着各种设计模式的应用，设计模式理解得越好，也越能理解UE的框架设计。  
言归正传，设计模式的本质就是抽象变化。如果依照纯朴的"程序=数据+算法"的结构来看，再算上用于用户显示和输入的界面，那么就得到“程序=数据+算法+显示”。这三大基本块（数据，算法，显示）构成了程序的三大变化，而如何把这三者“+”到一起，用的就是我们的种种设计框架模式。  
典型的，对于游戏：

- “显示”指的是游戏的UI，是屏幕上显示的3D画面，或是手柄上的输入和震动，也可以是VR头盔的镜片和定位，是与玩家直接交互的载体；
- “数据”指的是Mesh，Material，Actor，Level等各种元素组织起来的内存数据表示；
- “算法”可以是各种渲染算法，物理模拟，AI寻路，本文咱们就先暂时特指游戏开发者们编写的游戏业务逻辑。

抽象这三个变化，并归纳关系，就是典型的 [MVC模式](https://zhida.zhihu.com/search?content_id=1578067&content_type=Article&match_order=1&q=MVC%E6%A8%A1%E5%BC%8F&zhida_source=entity) 了：

![](https://pic2.zhimg.com/v2-6bfd369c2f163d0fd730bb7db8c5a7f9_1440w.jpg)

有些人可能会说MVC是UI专用的模式，如IOS的MVC或WPF的MVVM，也或者说因为游戏的类型千差万别所以一个通用的框架并不能都适用，因此就有一点点想要“返璞归真”的意味，觉得游戏引擎只需要提供一个基本的渲染框架，其他的逻辑框架不需要设计复杂，开发者们可自行根据游戏类型再设计。这种观点有一定的道理，对于简单的游戏或Demo，确实也还不到需要“设计”的地步；而对于复杂大型的游戏，需要的架构知识也确实远不是MVC这么简单。但缺点在于，说这话的人要嘛就已经是架构高手，各种设计模式信手拈来，早已经到了无招胜有招的地步；要嘛就是回避了问题，游戏也是软件，软件的固有复杂度摆在那里，总得需要个办法去解决，今天如果我们不是在探讨尝试用MVC模式去掌控它，也是在谈一个别的名字的模式。在我看来，一个好的游戏引擎，应该是能尽力的帮助用户，并减少麻烦。MV当然也有它的缺陷和不足，所以我们应该研究的是UE为何选择了MVC，有什么优点缺点，我们怎么利用和规避，让UE的Controller们尽责的为我们服务，少造成麻烦。  
对于简单的游戏或者引擎来说，有时并不需要把这三者分的很清，如Cocos2dx就没有Controller，它的MVC就是混杂在一起，因为代码量少所以也还算勉强能凑合；Unity的MonoBehavior其实也相当于把MC放在了一起，用得方便的同时也得小心太顺手了出现组件之间互相网状引用一团乱麻的情况；UE在这个问题上的思考就有些一脉相承，既然Actor们形形色色，我们之前也谈过甚至有AInfo这种书记官，那为何不让一些Actor专门来承载逻辑呢？于是，Actor再度分化出Controller。下面我们就来一一介绍Actor旗下Controller家族的指挥官们。

## AController

虽然我在之前已经一再的剧透过AController是继承自 [AActor](https://zhida.zhihu.com/search?content_id=1578067&content_type=Article&match_order=1&q=AActor&zhida_source=entity) 的一个子类，但是为了更好理解思考UE里的Controller机制，请先把脑袋放空，也别去偷看UE里的源码，像张无忌一样暂时忘记AController这回事，问自己一个问题：如果我想实现一种机制去控制游戏里的Actor，该怎么设计？  
巧妇难为无米之炊，咱们先来看看当前手上都有些什么：

1. **UObject** ，反射序列化等机制
2. **UActorComponent** ，功能的载体，一定程度的嵌套组装能力（SceneComponent）
3. **AActor** ，基础的游戏对象，Component的容器
4. **[APawn](https://zhida.zhihu.com/search?content_id=1578067&content_type=Article&match_order=1&q=APawn&zhida_source=entity)** ，分化出来的AActor，物理表示和基本的移动能力，当前正翘首以待。
5. **没了** ，在控制Actor这个层级，我们还暂时不需要去考虑Level等更高层次的对象

针对APawn，再想想我们希望达成的控制愿景，没事，你尽管放开想象的想，做不做得到咱们先放一边，但至少别在一开始就被想象力限制住了。“控制”本身虽然只是一段逻辑算法代码，但是它也需要有个载体去承载和运行，某种意义上来说也算得上是个实体。所以下面我们不妨就脑洞大开，以“控制”这个实体的视角口吻，讲讲“我，作为一个——控制”希望拥有哪一些本领：

1. **能够和Pawn对应起来** ，理想情况下，极端的灵活性应该是多对多。我希望我能同时控制多个Pawn，当然，一个Pawn也可以被多个我的兄弟姐妹们一起控制。想想那些RTS游戏和多人协作游戏，你应该能明白我有时候需要协调调度Pawn们走个方阵，有时候也得多人合作才能操纵得了一台机甲。当然越灵活也往往意味着越容易出错，但总之我们需要一个和Pawn关联的机制。
2. **多个控制实例** ，在需要的时候，我不介意可以克隆出多个我来，比如一段逻辑A，我们希望可以有多个实例在同时运行。就像 [行为树](https://zhida.zhihu.com/search?content_id=1578067&content_type=Article&match_order=1&q=%E8%A1%8C%E4%B8%BA%E6%A0%91&zhida_source=entity) 一样，可以有多个运行实例，彼此算法一样，但互不干扰。
3. **可挂载释放** ，我可以选择当前控制PawnA，也可以选择之后把它释放掉不再控制让她自生自灭，然后再另寻新欢控制PawnB，我必须拥有灵活的运行时增删控制Pawn的能力。
4. **能够脱离Pawn存在** ，我思故我在，就算当前没有任何Pawn控制，我也可以继续存在，这样我就可以延时动态的选择Pawn对象。有些Pawn值得我去等。
5. **操纵Pawn生死的能力** ，谁规定必须一定去控制世界当前存在的Pawn才行。当世界里没有Pawn可供我控制时，我希望可以自己造一个出来。你要说她是玩具、亦或傀儡也好，我不在乎。有时候我很羡慕暗黑里的沉沦魔巫师，身边总是围绕着一群沉沦魔，一个沉沦魔挂了，他可以紧接着再复活一个出来，这样永远都不会感动寂寞，你说多好？那索性再霸道一点吧，要是我这个控制实体不在了，我希望可以选择是否带Pawn们跟我一起走，没了我，她们都傻得让人心疼。当然如果有哪个Pawn能让我这个霸道总裁爱上，我也愿意陪她一起去死。
6. **根据配置自动生成** ，我（控制）虽然只是一段代码，但也不能无中生有，所以也得有个机制可以生成我这个控制实体，不过想来这应该是组织里更上层领导的事，但至少他应该知道怎么创建我出来。
7. **事件响应** ，游戏事件的一些控制关心的事件应该能够传到我这里，我可以酌情处理。同样，Pawn也可以向我汇报，我会好好研究决定的，嗯。
8. **持续的运行** ，没事的时候，我喜欢听世界大钟的每一次Tick，跟我的心跳同步起来，就仿佛真的活过来一样，可以自主的做一些我想做的事，这是我最自在的时候。
9. **自身有状态** ，你累了要休息，我也一样。我可以选择自身的状态，选择工作或者是休息，也可以选择今天是哪个Pawn和心情最配。
10. **拥有一定的扩展继承组合能力** ，一方面我希望我的家族开枝散叶繁荣昌盛，我的一身本领继承自我的父亲，而我也将有我的儿，大家各有天赋。另一方面，那些普通的Actor们都可以身背各个Component，更高贵的我当然也想有。
11. **保存数据状态** ，听说金鱼的记忆只有7秒，可是我却想记住你一辈子。所以我希望我能拥有一些记忆，人的过去成就了现在，也将指引着未来。以前有一个人跟我说过，当你不能再拥有的时候，唯一能做的就是令自己不要忘记。
12. **可在世界里移动** ，我可以选择帐中千里之外遥控Pawn，也可以选择附身在一个Pawn身上，这样我才能多角度无死角的观察我可爱的Pawn们，嘿嘿。
13. **可探查世界的对象** ，我要有眼睛，让我干活，基本的我得看得见知道当前世界里已经有哪些对象吧，否则不就抓瞎了嘛。
14. **可同步** ，这年头，要是不能适应网络环境，可真的没有竞争力。这个Object，Actor基本都有的能力，我当然也得有。位于服务器或客户端上的我也必须有能力在其他客户端上影分身，让他们都跟随我的步伐一致行动。

在仔细考察了"控制"的需求和手头上的原料之后，我们试着从UE的角度来权衡一下。  
首先Controller不能是一个Component，一是因为Component的层级太低，表达的是功能的概念而非逻辑；二是Component必须依附于Actor存在，而我们的Controller希望能独立存在。  
其次如果从UObject直接继承下来UController，倒是也可行，UObject也能复制同步，其他的控制Pawn的能力和事件响应等倒也是能改改接口想想办法，但是要想在世界里移动，就得有个位置表示，再加上还希望能容纳Components，这就麻烦了，基本就是把Actor的工作再做一套，有点累人，搞起来也怕两套班子出错闹矛盾。  
再来考察下从AActor继承下来AController怎么样，Actor比Object多了一些我们正需要的配置动态生成、输入事件响应、Tick、可继承、可容纳Component、可在世界里出现、可在网络间同步。好了，现在就差控制Pawn的能力，那我们就在这个分化出来的AController增加一些控制Pawn的接口，这个思路正是和我们从Actor从分化出Pawn的时候不谋而合！现在我们来看看UE里的AController:  

![](https://pic4.zhimg.com/v2-4151952d1f2ab74fcc78d7c3bd215e0d_1440w.jpg)

跟我们的设计八九不离十，我们再一一仔细验证一番：  
关联Pawn的能力，有Possess和UnPossess，源码里也有PawnPendingDestroy等这些函数（未一一列出）；GameMode中也保存着AIControllerClass和PlayerControllerClass的配置，用于在适当的时候Spawn出Controller；继承于Actor也就有了EnableInput和Tick；Controller本身还可以继续派生下去（如AIController和PlayerController），也可以容纳Components；也带着一个SceneComponent所以可以摆放在世界中；自身也可以添加成员变量来记忆存储游戏状态；自身也有一个FName StateName（Playing、Spectating、Inactive），切换自身的状态（运行，观察，非激活）；因为跟Pawn是平级的关系，只在运行的时候引用关联，所以对彼此独立存在不做强制约束，提高了灵活性。一个Pawn自身上也可以配置策略：

```cpp
namespace EAutoReceiveInput
{
    enum Type
    {
        Disabled,
        Player0,
        Player1,
        Player2,
        Player3,
        Player4,
        Player5,
        Player6,
        Player7,
    };
}
TEnumAsByte<EAutoReceiveInput::Type> AutoPossessPlayer;
enum class EAutoPossessAI : uint8
{
    /** Feature is disabled (do not automatically possess AI). */
    Disabled,
    /** Only possess by an AI Controller if Pawn is placed in the world. */
    PlacedInWorld,
    /** Only possess by an AI Controller if Pawn is spawned after the world has loaded. */
    Spawned,
    /** Pawn is automatically possessed by an AI Controller whenever it is created. */
    PlacedInWorldOrSpawned,
};
EAutoPossessAI AutoPossessAI;
TSubclassOf<AController> AIControllerClass;
```

这样在运行时UE也可以根据Pawn创建配套的Controller。毕竟只是为了阐明概念，而不是纠结技术细节，我对Controller的功能接口都只是粗略带过，如果读者自己去看Contoller的UE源码，顺便可以对我当前说的概念验证一下，还会发现一些Movement和ViewPoint的接口，这些也算是和控制移动和视角配套吧。  

**思考：Controller和Pawn必须1:1吗？**  
观察UE实现里我们发现Controller里只是保存了一个Pawn指针，而不是数组，这和一开始希望的多对多关系有些出入。理想和现实总是有差距，一个愿景落实到工程实践上也不免得有一些妥协。首先我们再来梳理理解一下这个Possess(拥有/占用)的概念。一个Controller能灵活的Possess/UnPossess一个Pawn，虽然一次只能控制一个，但在游戏中我们也可以在不同的Pawn中切换，比如操纵一个主角坐进然后控制一辆汽车，或者端起固定的机关枪扫射，这些功能琢磨一下其实只是涉及操作实体Pawn的变化。如果我们能妥善的用好Pawn和Controller的切换功能，大部分基本的游戏功能也是能够比较方便的实现的。那么有哪些是不太适合的呢？UE官方其实也承认了，见 [Controller](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/latest/INT/Gameplay/Framework/Controller/index.html) 文档说明：

> By default, there is a one-to-one relationship between Controllers and Pawns; meaning, each Controller controls only one Pawn at any given time. This is acceptable for most types of games, but may need to be adjusted as certain types of games - real-time strategy comes to mind - may require the ability to control multiple entities at once.

对于RTS这种需要一下子控制多个单位的游戏来说，这种1v1的关系确实比较僵硬，就需要在Controller里自己实现扩展一下，额外保存多个Pawn，然后自己实现一些需要的控制实现，但总体上也只能说得绕一下，也算不上特别复杂，所以就也不能说UE做不了某一些类型的游戏，Epic是个游戏引擎公司，卖的毕竟是个通用游戏引擎。  
OK，那UE为何不实现成多对多呢？我觉得理由往往很简单，就是想保持一定的简单。游戏引擎的每个模块的设计，甚至函数接口的设计，无时无刻不在权衡决定。太简单了概念清晰用起来方便但是灵活扩展力不足，太灵活扩展无限了往往也会让人无从适从容易出错。当前1:1的时候，我们的脑袋逻辑很清晰，我们可以在Controller里直接GetPawn，也可以在Pawn中GetController，都非常方便。调试逻辑Bug的时候，我们也能很快找到查错的目标。而对比想象，如果是M：N，灵活性是满满了，但是你能轻易的说出当前Pawn是被哪些Controller控制吗？你也得时时记着这个Controller当前控制了哪些Pawn。OMG！这些Pawn和Controller多对多的构成了网状结构，项目越庞大复杂，这张网也越能套住你。再从另一个方面说，一旦提供了这种多对多的直接支持，以我们人类的性格，免费现成的东西，我们总是倾向于去找机会能用上它，而不是去琢磨到底应不应该用。所以一旦就这么直接提供了，对于刚入门的新手，压根就没什么指引，怎么来好像都可以，就非常容易收不住把项目逻辑关系搞得不必要的复杂。所以以后UE就算想在这一方面优化加强，应该也会比较克制。  
索性再聊开一些，我们用Unity来做一下对比。Unity就是GameObject+Component，你自己组合去吧，非常的灵活自由，也不做什么限制，但造成的后果就是常常各种Component互相引用来引用去，网状互联一团乱麻。另外几乎每个人都可以在上面搞出一套游戏系统出来，互相之间都是自成一派。所以经常网上就会有各种帖子问怎么在Unity中实现MVC模式的，也有分析炉石传说游戏逻辑框架的。Unity当然是个好引擎，目前来说热度也是比UE要高一些，但我们也不能因为它火用得人多，就权威崇拜从众的认为Unity各个方面都比别的引擎好。设计架构游戏的时候，工程师们要抵挡住灵活性的诱惑，保持克制往往是更难得珍贵的美德。要认识到，引擎的终极目的是方便人使用的，我们程序员往往很容易太沉迷于程序功能的灵活强大，而疏忽了易用性鲁棒性等社会工程需求。

**思考：为何Controller不能像Actor层级嵌套？**  
我们都知道Actor可以藉着身上的SceneComponent互相嵌套。那么AController同样也是Actor，为何不也实现这么一个父子机制？从功能上来说，一个Controller可以有子Controllers，听起来也是非常灵活强大啊。但是冷静想一下，Controller表达的“控制”的概念，所以在这里你实际上想要表达的是一种“控制”互相嵌套的概念，感觉又给“控制”给分了层，有“大控制”，也有“小控制”，但是“控制”的“大小”又是个什么概念呢？我们应该怎么划分控制的大小？“控制”本质上来说就是一些代码，不管怎么设计，目的都是用来表达游戏游戏逻辑的。而针对游戏逻辑的复杂，怎么更好的管理组织逻辑代码，我们有状态机，分层状态机，行为树，GOAL（目标导向），甚至你还能搞些神经网络遗传算法机器学习啥的。所以在我们已经有这么多工具的基础上，徒增复杂性是很危险的做法。如果有必要，也可以把Controller本身再当作其他AI算法的容器，所以就没必要在对象层次上再做文章了。

**思考：Controller可以显示吗？**  
既然Actor本身可以带着Mesh组件来渲染显示，那Controller可不可以呢？是不是Controller都是不可见的？这个答案可说是也可以说不是，因为Controller本身确实就是一个特殊点的Actor而已，你依然可以在Controller中添加Mesh组件，添加别的子Actor等，所以从这个方面说Controller是有可以渲染显示的能力的。但是一个控制者毕竟只是表达一个逻辑的概念，所以为了分工明确，UE就干脆在Controller的构造函数里把自己给隐藏了：

```cpp
bHidden = true;
#if WITH_EDITORONLY_DATA
    bHiddenEd = true;
#endif // WITH_EDITORONLY_DATA
```

事了拂衣去，深藏功与名。为了验证我的说法，读者你可以亲自在PlayController下挂一些Cube之类的Actor，然后在源码层把这两个值改为false，重新编译运行看下结果，看能否正确显示出来，这里我就不贴图了，留给读者验证，很好玩的哦。  

**思考：Controller的位置有什么意义？**  
既然Controller本身只是控制者，那它在场景中的位置和移动有什么意义吗？Controller为何还需要个SceneComponent?意义在于如果Controller本身有位置信息，就可以利用该信息更好的控制Pawn的位置和移动。  
首先说下Controller的Rotation，这个比较好理解一点，如果我想让我的Pawn和Controller保持旋转朝向一致，因为是Controller作主控制Pawn的关系，所以Controller就得维护自己的Rotation。再来说位置，如果Controller有自己的位置，这样在Respawn重新生成Pawn的时候，你就可以选择在当前位置创建。因此为了自动更新Controller的位置，UE还提供了一个bAttachToPawn的开关选项，默认是关闭的，UE不会自动的更新Controller的位置信息；而如果打开，就会把Controller附加到Pawn的子节点里面去，让Controller跟随Pawn来移动。你可以把这两种模式想象成一种是上帝视角在千里之外心电感应控制Pawn，另一种是骑在Pawn肩上来指挥。  
当然如果这个Controller确实只是纯朴的逻辑控制的话（如AIController），那确实位置也没什么意义。所以UE甚至还隐藏了Controller的一些更新位置的接口，尽量避免让人手动去操纵：

```
private:
    // Hidden functions that don't make sense to use on this class.
    HIDE_ACTOR_TRANSFORM_FUNCTIONS();
//展开后：
//////////////////////////////////////////////////////////////////////////
// Macro to hide common Transform functions in native code for classes where they don't make sense.
// Note that this doesn't prevent access through function calls from parent classes (ie an AActor*), but
// does prevent use in the class that hides them and any derived child classes.
#define HIDE_ACTOR_TRANSFORM_FUNCTIONS() private: \
    FTransform GetTransform() const { return Super::GetTransform(); } \
    FVector GetActorLocation() const { return Super::GetActorLocation(); } \
    FRotator GetActorRotation() const { return Super::GetActorRotation(); } \
    FQuat GetActorQuat() const { return Super::GetActorQuat(); } \
    FVector GetActorScale() const { return Super::GetActorScale(); } \
    bool SetActorLocation(const FVector& NewLocation, bool bSweep=false, FHitResult* OutSweepHitResult=nullptr) { return Super::SetActorLocation(NewLocation, bSweep, OutSweepHitResult); } \
    bool SetActorRotation(FRotator NewRotation) { return Super::SetActorRotation(NewRotation); } \
    bool SetActorRotation(const FQuat& NewRotation) { return Super::SetActorRotation(NewRotation); } \
    bool SetActorLocationAndRotation(FVector NewLocation, FRotator NewRotation, bool bSweep=false, FHitResult* OutSweepHitResult=nullptr) { return Super::SetActorLocationAndRotation(NewLocation, NewRotation, bSweep, OutSweepHitResult); } \
    bool SetActorLocationAndRotation(FVector NewLocation, const FQuat& NewRotation, bool bSweep=false, FHitResult* OutSweepHitResult=nullptr) { return Super::SetActorLocationAndRotation(NewLocation, NewRotation, bSweep, OutSweepHitResult); } \
    virtual bool TeleportTo( const FVector& DestLocation, const FRotator& DestRotation, bool bIsATest, bool bNoCheck ) override { return Super::TeleportTo(DestLocation, DestRotation, bIsATest, bNoCheck); } \
    virtual FVector GetVelocity() const override { return Super::GetVelocity(); } \
    float GetHorizontalDistanceTo(AActor* OtherActor)  { return Super::GetHorizontalDistanceTo(OtherActor); } \
    float GetVerticalDistanceTo(AActor* OtherActor)  { return Super::GetVerticalDistanceTo(OtherActor); } \
    float GetDotProductTo(AActor* OtherActor) { return Super::GetDotProductTo(OtherActor); } \
    float GetHorizontalDotProductTo(AActor* OtherActor) { return Super::GetHorizontalDotProductTo(OtherActor); } \
    float GetDistanceTo(AActor* OtherActor) { return Super::GetDistanceTo(OtherActor); }
```

UE这里其实想说的是，这些更新位置的操作还是让我来为你管理吧，我真的担心你会用错搞出什么乱子来。顺便再说些题外话，对于PlayerController来说，因为玩家需要一个视角来观察世界，所以常常PlayerController常常会扛着个摄像机出现（蓝图里没有，但是会运行时生成PlayerCameraManager和CameraActor），所以就算没有角色可供操作，玩家也依然希望是可以视角漫游观察整个世界的（试试看把默认Level里的PlayerStart删掉后运行看看）。这个时候PlayerController常常会默认创建出一个ASpectatorPawn或者DefaultPawn（根据GameMode里配置），我们虽然看不见Pawn，但依然可以观察世界，靠得就是跟Controller关联的旋转和摄像机。  

**思考：哪些逻辑应该写在Controller中？**  
如同当初我们在思考Actor和Component的逻辑划分一样，我们也得要划分哪些逻辑应该放在Pawn中，哪些应该放在Contrller中。上文我们也说过，Pawn也可以接收用户输入事件，所以其实只要你愿意，你甚至可以脱离Controller做一个特立独行的Pawn。那么在那些时候需要Controller？哪些逻辑应该由Controller掌管呢？可以从以下一些方面考虑：

- 从概念上，Pawn本身表示的是一个“能动”的概念，重点在于“能”。而Controller代表的是动到“哪里”的概念，重点在于“方向”。所以如果是一些Pawn本身固有的能力逻辑，如前进后退、播放动画、碰撞检测之类的就完全可以在Pawn内实现；而对于一些可替换的逻辑，或者智能决策的，就应该归Controller管辖。
- 从对应上来说，如果一个逻辑只属于某一类Pawn，那么其实你放进Pawn内也挺好。而如果一个逻辑可以应用于多个Pawn，那么放进Controller就可以组合应用了。举个例子，在战争游戏中，假设说有坦克和卡车两种战车（Pawn），只有坦克可以开炮，那么开炮这个功能你就可以直接实现在坦克Pawn上。而这两辆战车都有的自动寻找攻击目标功能，就可以实现在一个Controller里。
- 从存在性来说，Controller的生命期比Pawn要长一些，比如我们经常会实现的游戏中玩家死亡后复活的功能。Pawn死亡后，这个Pawn就被Destroy了，就算之后再Respawn创建出来一个新的，但是Pawn身上保存的变量状态都已经被重置了。所以对于那些需要在Pawn之外还要持续存在的逻辑和状态，放进Controller中是更好的选择。

## APlayerState

我们上文提到过Controller希望也能有一些记忆，保存住一些游戏状态。那么到底应该怎么保存呢？AController自身当然可以添加成员变量来保存，这些变量也可以网络复制，一般来说也够用。但是终究还是遗忘了一个最重要的数据状态。整个游戏世界构建起来就是为了玩家服务的，而玩家在游戏过程中，肯定要存取产生一些状态。而Controller作为游戏业务逻辑最重要的载体，势必要和玩家的状态打交道。所以Controller如果可以动态存取玩家的状态就会大为方便了。因此我们会在Controller中见到：

```
/** PlayerState containing replicated information about the player using this controller (only exists for players, not NPCs). */
   UPROPERTY(replicatedUsing=OnRep_PlayerState, BlueprintReadOnly, Category="Controller")
   class APlayerState* PlayerState;
```

而APlayerState的继承体系是：  

![](https://pic3.zhimg.com/v2-ba203b15c1e9356d5aa7fe6bf2fd556c_1440w.jpg)

至于为啥APlayerState是从AActor派生的AInfo继承下来的，我们聪明的读者相信也能猜得到了，所以也就不费口舌论证了。无非就是贪图AActor本身的那些特性以网络复制等。而AInfo们正是这种不爱表现的纯数据书呆子们的大本营。而这个PlayerState我们可以通过在GameMode中配置的PlayerStateClass来自动生成。  
注意，这个APlayerState也理所当然是生成在Level中的，跟Pawn和Controller是平级的关系，Controller里只不过保存了一个指针引用罢了。注释里说的PlayerState只为players存在，不为NPC生成，指的是PlayerState是跟UPlayer对应的，换句话说当前游戏有多少个真正的玩家，才会有多少个PlayerState，而那些AI控制的NPC因为不是真正的玩家，所以也不需要创建生成PlayerState。但是UE把PlayerState的引用变量放在了Controller一级，而不是PlayerController之中，说明了其实AIController也是可以设置读取该变量的。一个AI智能能够读取玩家的比分等状态，有了更多的信息来作决策，想来也没有什么不对嘛。  
Controller和网络的结合很紧密，很多机制和网络也非常强关联，但是在这里并不详细叙述，这里先可以单纯理解成Controller也可以当作玩家在服务器上的代理对象。把PlayerState独立构成一个Actor还有一个好处，当玩家偶尔因网络波动断线，因为这个连接不在了，所以该Controller也失效了被释放了，服务器可以把对应的该PlayerState先暂存起来，等玩家再紧接着重连上了，可以利用该PlayerState重新挂接上Controller，以此提供一个比较顺畅无缝的体验。至于AIController，因为都是运行在Server上的，Client上并没有，所以也就无所谓了。

**思考：哪些数据应该放在PlayerState中？**  
从应用范围上来说，PlayerState表示的是玩家的游玩数据，所以那些关卡内的其他游戏数据就不应该放进来（GameState是个好选择），另外Controller本身运行需要的临时数据也不应该归PlayerState管理。而玩家在切换关卡的时候，APlayerState也会被释放掉，所有PlayerState实际上表达的是当前关卡的玩家得分等数据。这样，那些跨关卡的统计数据等就也不应该放进PlayerState里了，应该放在外面的GameInstance，然后用SaveGame保存起来。

## 总结

在游戏里，如果要评劳模，那Controller们无疑是最兢兢业业的，虽然有时候蛮横霸道了一些，但是经常工作在第一线，下面的Pawn们常常智商太低，上面的Level，GameMode们又有点高高在上，让他们直接管理数量繁多的Pawn们又有点太折腾，于是事无巨细的真正干那些脏活累活的还得靠Controller们。本文虽然没有在网络一块留太多笔墨，但是Controller也是同时作为联机环境中最重要的沟通渠道，身兼要职。  
回顾总结一下本文要点，UE在Pawn这个层级演化构成了一个最基本和非常完善的Component-Actor-Pawn-Controller的结构：

![](https://pic3.zhimg.com/v2-117fa2fe09c46ed2dac388278f028df0_1440w.jpg)

通过分化出来后的Actor的互相控制，既充分利用了现有的机制功能，又提供了足够的灵活性，而且做的更改还很少，不用再设计额外另一套框架。读者朋友们，现在我们如果翻到第一小节，想想UE最初从Object分化出Actor的那一刻，是不是有很多感慨和感动呢？一个最初的很简单的游戏对象表示，慢慢演化派生充实起来，彼此之间通力配合，竟也能优雅的运转起来。

有时候架构的设计和搭建是一脉相承的，最初的时候选择了什么样的模型和骨架，后面再设计别的逻辑框架等其他模块，也基本上都得跟最初的设计配合着来。所以有时候往往也会发现，怎么感觉我架构设计的方案可选择数量并不多啊？其实是因为如果一开始铺垫的好，接下来的设计水到渠成自然而然，让你感觉不到用心设计的力气。UE以Actor的视角来看待世间万物，自然得到的是一个Actor繁荣昌盛的世界；Unity以Component来组装万物，得到的就是个各种插件组件组装出的世界；而如果如Cocos2dx一般万物都是Node,那么自然也会得到一棵挂满各种Node的世界之树。这也算是游戏引擎的基因吧。

本想着一篇介绍完Controller、PlayerController和AIController这三个对象，但是Controller本身是UE里极为重要的核心概念，自身的功能非常的丰富，牵扯的模块也比较多，因此想抽离阐述最核心的概念和功能并不是一件容易的事。花了这么长的篇幅，只讨论揣摩了Controller的设计过程和最基本的职责（还有输入网络等都没有解释），顺便先简单介绍了下PlayerState出场（PlayerState实际上是跟UPlayer关联更大一些，PlayerController等后续章节会继续讨论它），对于PlayerController和AIController，目前也只是语焉不详的含糊带过。不过还是希望读者们能从中吸取到设计的营养，把握清楚概念了，才能更好的组织游戏逻辑，开发出更好的游戏。

本系列教程的一个重点也是尝试介绍引擎各种概念背后的考量，而不是单纯的叙述解释各个模块功能。笔者始终认为，只有我们愿意不吝口舌的去讨论，愿意耐下心来去思考学习，这些概念的领悟才会了然在心中。否则若只是单纯的介绍Pawn功能有123，Controller可以ABC，相信读者在阅读完之后也并不会有什么深的印象，因为这些只是设计的结果，少了设计的过程。

上篇： [《InsideUE4》GamePlay架构（四）Pawn](https://zhuanlan.zhihu.com/p/23321666?refer=insideue4)  

下篇我们将隆重介绍Controller家族中最耀眼的明星、上帝的宠儿： [《InsideUE4》GamePlay架构（六）PlayerController和AIController](https://zhuanlan.zhihu.com/p/23649987) ！

## 引用

1. [Controller](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/latest/INT/Gameplay/Framework/Controller/index.html)

UE4的版本更新实在太快，为了留下版本存照和供读者查证，以后在篇尾都会标注上本文研究使用的源码版本。以后不再特意做此声明。  
*UE 4.13.2*

\---------------------------------------------------------------------------------------------------------------------------

知乎专栏： [InsideUE4](https://zhuanlan.zhihu.com/insideue4)

UE4深入学习QQ群： **456247757** (非新手入门群，请先学习完官方文档和视频教程)

微信公众号： **aboutue** ，关于UE的一切新闻资讯、技巧问答、文章发布，欢迎关注。

**个人原创，未经授权，谢绝转载！**

2 人已送礼物

编辑于 2022-03-18 18:50[游戏引擎](https://www.zhihu.com/topic/19556258)[虚幻引擎](https://www.zhihu.com/topic/19824201)[游戏开发](https://www.zhihu.com/topic/19553361)

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
| K1 | 引言 | Controller 是 UE 中承载“控制/业务逻辑”的关键层。 | 无 |
| K2 | MVC | MVC 的核心是把数据、算法和显示解耦。 | K1 |
| K3 | MVC | 游戏中的显示、数据、算法分别对应不同的抽象对象。 | K2 |
| K4 | AController | AController 从 Actor 继承，是为了复用世界中的存在性与调度能力。 | K1-K3 |
| K5 | AController | Controller 的设计需求决定了它必须既能独立存在，又能绑定 Pawn。 | K4 |
| K6 | 思考 | UE 选择 1:1 Controller-Pawn 关系，是为了简化认知和调试。 | K5 |
| K7 | 思考 | Controller 不适合做层级嵌套，因为“控制”本身没有清晰的大小层级。 | K5-K6 |
| K8 | 思考 | Controller 的位置、隐藏和视角管理，是为了把逻辑控制与表现隔离。 | K4-K7 |
| K9 | APlayerState | PlayerState 用来承接玩家状态，而不是把所有状态都堆在 Controller。 | K8 |
| K10 | 总结 | Controller 是连接 Pawn、输入、状态与控制逻辑的中枢层。 | K1-K9 |

### [AI注释-新增][K1] Controller 的核心位置

- 核心句：Controller 是 UE 用来承载“控制逻辑”的专门层。
- 为什么重要：如果不把控制逻辑单独拎出来，Actor 和 Pawn 会越来越像业务杂物间。
- 机制解释：
- L1 直觉层：Pawn 是棋子，Controller 是下棋的人。
- L2 机制层：Controller 主要处理“怎么控制”的问题，而不是单纯“被显示”的问题。
- L3 工程层：它把输入、决策、控制、状态这些逻辑从表现实体中剥离出来。
- 思考题：为什么 UE 不把“控制逻辑”直接塞进 Pawn 或 Actor 就完事？
- 考察重点：是否能理解 Controller 是专门为“控制职责”建立的独立层。
- 参考答案：因为 Pawn/Actor 主要表达世界中的实体和表现，如果把控制逻辑也塞进去，职责会混在一起；Controller 作为独立层，能更清楚地处理输入、决策和控制关系，也更方便替换和复用。

### [AI注释-新增][K2] MVC 的本质

- 核心句：MVC 的本质是把数据、算法和显示分开管理。
- 为什么重要：这是理解 UE 为什么要分出 Controller 的理论入口。
- 机制解释：
- L1 直觉层：把“看得见的东西”“存的数据”“做决定的逻辑”分开。
- L2 机制层：不同部分的变化频率和关注点不同，混在一起会让维护成本升高。
- L3 工程层：游戏的 UI、对象数据和业务逻辑各自应该有清晰边界。
- 思考题：程序=数据+算法+显示，这三者为什么不能全都写在一个类里？
- 考察重点：是否能说出分层的目的在于管理复杂度。
- 参考答案：因为这三者变化原因不同、使用方式不同、测试方式也不同；如果塞在一个类里，代码会同时受到显示、状态和逻辑变化影响，难以维护和扩展。

### [AI注释-新增][K3] 游戏里的三类变化

- 核心句：在游戏里，“显示”“数据”“算法”对应三类不同的变化源。
- 为什么重要：只有分清这三类变化，才能把模式选对。
- 机制解释：
- L1 直觉层：屏幕上看到的是显示，内存里存的是数据，真正做决定的是算法。
- L2 机制层：Mesh、Material、Actor、Level 更偏数据；UI 和3D画面更偏显示；业务逻辑属于算法。
- L3 工程层：MVC 等框架就是围绕这三者做职责切分。
- 思考题：为什么游戏开发里“显示”和“数据”不能简单看作同一件事？
- 考察重点：是否能区分视觉呈现与内存结构。
- 参考答案：因为显示关注的是玩家看到什么、如何反馈；数据关注的是对象在内存中的组织和状态。二者虽然相关，但一个负责表现，一个负责存储和结构，不应混同。

### [AI注释-新增][K4] 为什么 AController 继承自 AActor

- 核心句：Controller 需要复用 Actor 的世界存在性、Tick、输入和可容纳组件等能力。
- 为什么重要：如果直接从 UObject 做 Controller，就得重复造一整套世界级能力。
- 机制解释：
- L1 直觉层：控制者也得“站在世界里”，不是只会算数的纯工具类。
- L2 机制层：AActor 提供了可在世界中存在、可 Tick、可输入、可扩展的基础。
- L3 工程层：Controller 需要这些能力才能绑定 Pawn、接收事件和参与运行时调度。
- 思考题：如果 Controller 不从 Actor 继承，会缺掉哪些关键能力？
- 考察重点：是否能说出“世界存在性、Tick、输入、组件、网络同步”等基础能力。
- 参考答案：会缺少在世界中存在、Tick、输入事件、组件挂载、位置表示和一定的网络协作能力，最后还得重写一遍大部分 Actor 的基础设施，成本太高。

### [AI注释-新增][K5] Controller 的需求画像

- 核心句：Controller 需要独立存在，又要能绑定 Pawn，还要能承载状态和事件。
- 为什么重要：这个需求画像解释了为什么 UE 没把它放成简单函数或组件。
- 机制解释：
- L1 直觉层：它像导演，能临时换演员，但导演自己要持续存在。
- L2 机制层：Controller 既要 Possess/UnPossess Pawn，也要持续 Tick、保存状态、接收事件。
- L3 工程层：它是连接“控制意图”和“受控实体”的桥梁。
- 思考题：为什么 Controller 不能只做成一个纯函数集合？
- 考察重点：是否能理解 Controller 是运行时实体，而不是一次性工具函数。
- 参考答案：因为控制逻辑需要持续存在、维护状态、响应事件、切换 Pawn，并在世界中参与运行；纯函数集合无法保存这些上下文和生命周期状态。

### [AI注释-新增][K6] 为什么 UE 选择 1:1 Controller-Pawn

- 核心句：UE 用 1:1 关系简化了理解、调试和默认使用路径。
- 为什么重要：这解释了为什么很多复杂场景必须自己扩展，而不是系统直接放开。
- 机制解释：
- L1 直觉层：一把钥匙先开一把锁，规则最清楚。
- L2 机制层：Controller 里直接保存一个 Pawn 指针，Pawn 也能反查 Controller。
- L3 工程层：这样调试链路短，默认行为简单，适合大多数游戏。
- 思考题：为什么 RTS 这种游戏会觉得 1:1 不够用？
- 考察重点：是否能理解 1:1 的好处是简单、清晰、易调试，而不是“功能最强”。
- 参考答案：RTS 需要一个控制者同时指挥多个单位，而 1:1 默认只能控制一个 Pawn，所以会显得僵硬；不过 UE 允许开发者在 Controller 上自行扩展多个 Pawn 的管理逻辑，只是默认不帮你走复杂路线。

### [AI注释-新增][K7] 为什么不做 Controller 嵌套

- 核心句：控制没有清晰的“大小层级”，层层嵌套只会让关系更难理解。
- 为什么重要：这背后其实是在控制架构复杂度。
- 机制解释：
- L1 直觉层：控制就是安排事情，没必要再生出“大控制、小控制”一层层套娃。
- L2 机制层：控制本身可以通过状态机、行为树、目标导向等算法来表达复杂度。
- L3 工程层：如果再把 Controller 也做成层级结构，网状依赖会更严重。
- 思考题：为什么 UE 更愿意让控制算法自己分层，而不是让 Controller 对象层层嵌套？
- 考察重点：是否能把“算法层次”和“对象层次”区分开。
- 参考答案：因为控制的复杂度更适合通过状态机、行为树、规则系统等算法结构去表达，而不是通过 Controller 对象层级去表达；对象层级一旦嵌套过深，会导致关系不清和调试困难。

### [AI注释-新增][K8] Controller 的位置、隐藏和视角

- 核心句：Controller 可以存在于世界中，但默认会隐藏自己，把表现交给更合适的对象。
- 为什么重要：这体现了 UE 对“逻辑实体”和“可见实体”分离的态度。
- 机制解释：
- L1 直觉层：导演不一定要站到舞台中央。
- L2 机制层：Controller 继承 Actor，所以也能放置、移动、挂组件；但默认会隐藏。
- L3 工程层：PlayerController 常会借助 Pawn 和 CameraManager 组织视角，而不是自己裸奔显示。
- 思考题：为什么 Controller 默认要隐藏？
- 考察重点：是否能联系“控制者”和“表现者”的分工。
- 参考答案：因为 Controller 的主要职责是控制而不是表现；如果默认可见，会把逻辑实体和表现实体混在一起，容易误导开发者。默认隐藏能提醒你它是逻辑层角色，而非视觉角色。

### [AI注释-新增][K9] PlayerState 为什么要独立出来

- 核心句：PlayerState 用来保存玩家状态，而不是把所有状态都堆在 Controller 里。
- 为什么重要：这关乎玩家状态、断线重连和网络同步的设计。
- 机制解释：
- L1 直觉层：玩家的信息应该放在“玩家档案”里，而不是临时指挥台上。
- L2 机制层：PlayerState 与玩家状态关联更紧，且可被复制、保存和复用。
- L3 工程层：当 Controller 失效或切换时，PlayerState 还能帮助恢复玩家状态。
- 思考题：哪些数据适合放在 PlayerState，哪些不适合？
- 考察重点：是否能区分“玩家级数据”“关卡局部数据”“控制器临时数据”。
- 参考答案：玩家得分、昵称、战绩、当前玩家状态等适合放 PlayerState；当前关卡临时目标、AI 决策缓存、控制器运行中的瞬态状态则不适合，因为它们生命周期和作用域不同。

### [AI注释-新增][K10] 本篇结论

- 核心句：Controller 是连接 Pawn、输入、状态与控制逻辑的中枢层。
- 为什么重要：它把“谁被控制”“怎么控制”“控制者保存什么状态”这些问题串起来了。
- 机制解释：
- L1 直觉层：Pawn 是棋子，Controller 是下棋的人。
- L2 机制层：Controller 管理控制关系、输入响应、状态与切换。
- L3 工程层：它是 UE Gameplay 架构里最核心的控制枢纽之一。
- 思考题：如果没有 Controller，Pawn 的控制逻辑会被挤到哪里？
- 考察重点：是否能总结 Controller 在整体架构中的位置，而不是只记接口。
- 参考答案：如果没有 Controller，控制逻辑很可能直接塞进 Pawn 或 Actor，导致控制与表现混杂、状态管理分散、复用困难。Controller 的存在就是为了把这些职责集中到一个更合适的中枢层里。

### [AI注释-新增] 单篇文章通过规则（执行版）

1. 每个 K 点按 0-5 分评分，低于 3 分视为未通过。
2. 未通过点必须补做验证任务并补交证据。
3. 全文通过条件：K 点通过率 >= 80%，且关键点 K4/K6/K8/K9 均 >= 3 分。
4. 若连续 2 个点未通过，回退到上一个通过点重做 L1-L2 讲解。

### [AI注释-新增] 自测记录模板

| 日期 | 文章 | K点 | 得分(0-5) | 证据摘要 | 结论 | 下一步 |
| --- | --- | --- | --- | --- | --- | --- |
| YYYY-MM-DD | GamePlay架构（五）Controller | K1 | 0-5 | 例如：Controller职责对照表 | 通过/未通过 | 进入K2或补测 |

<!-- AI注释-新增结束 -->