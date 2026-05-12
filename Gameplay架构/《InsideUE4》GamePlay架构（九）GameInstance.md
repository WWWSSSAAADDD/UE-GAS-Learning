---
title: "《InsideUE4》GamePlay架构（九）GameInstance"
source: "https://zhuanlan.zhihu.com/p/24005952"
author:
  - "[[大钊UE源码剖析，等编译的时候回答问题]]"
published:
created: 2026-04-18
description: "一人之下，万人之上引言上篇我们讲到了UE在World之上，继续抽象出了Player的概念，包含了本地的ULocalPlayer和网络的UNetConnection，并以此创建出了World中的PlayerController，从而实现了不同的玩家模式策略。一…"
tags:
  - "clippings"
---
> 一人之下，万人之上

## 引言

上篇我们讲到了UE在World之上，继续抽象出了Player的概念，包含了本地的ULocalPlayer和网络的UNetConnection，并以此创建出了World中的PlayerController，从而实现了不同的玩家模式策略。一路向上，依照设计里一个最朴素的原理：自己是无法创建管理自身的，所以Player也需要一个创建管理和存储的地方。另一方面，上文提到Player固然可以负责一些跟玩家相关的业务逻辑，但是对于World之上协调管理的逻辑却也仍然无处安放。  
如果是有一定的游戏开发实战经验的朋友也一定能体会到，在自己开发的游戏中，往往除了我们上文提到的Player类，常常会创建一个Game类，比如BattleGame、WarGame或HappyGame等等。Game之前的名词往往都是游戏的开发代号。这倒不是因为我们如此热衷创建各种Manager类，而是确实需要一个大管家来干一些协调的活。一般的游戏引擎都只会暴露给你它自己引擎的管理类，如Director，Engine或Application之类的，但是却不会主动在Game类的创建管理上为你提供方便。游戏引擎的出现，最开始其实只是因为一些人发现游戏做着做着，有一大部分功能是可以复用的，于是就把它抽离了出来方便做下一款游戏。在那个时候，人们对游戏还是处于开荒探索的阶段，游戏引擎只是一大堆功能的复合体，就像叮当猫的口袋一样，互相比谁掏出的工具最强大。然而即使到了现代，绝大部分的引擎的思想却还停留在上个世纪，仍然执着于罗列Feature列表，却忘了真正的游戏开发人员天天面对的游戏业务逻辑编写，没有思考在那方面如何也下一番功夫去帮助开发者。人们对比UE和其他游戏引擎时，也会常常说出的一句话是：“别忘了Epic自己也是做游戏的”（虚幻竞技场，战争机器，无尽之剑……）。从这一点也可以看出，UE很大的得益于Epic实战游戏开发的反哺，这一方面Unity就有点吃亏了，没有自己亲自下手干脏活累活，就不懂得急人民群众之所急。所以如果一个游戏引擎能把GamePlay也做好了，那就不止是口袋了，而是知你懂你的叮当猫本身。

## GameInstance

简单的事情就不用多讲了，UE提供的方案是一以贯之的，为我们提供了一个GameInstance类。为了受益于UObject的反射创建能力，直接继承于UObject，这样就可以依据一个Class直接动态创建出来具体的GameInstance子类。  

![](https://pica.zhimg.com/v2-e3572a2d46608304109104e6174688d8_1440w.png)

我并不想罗列所有的接口，UGameInstance里的接口大概有4类：

1\. 引擎的初始化加载，Init和ShutDown等（在引擎流程章节会详细叙述）

2\. Player的创建，如CreateLocalPlayer，GetLocalPlayers之类的。

3\. GameMode的重载修改，这是从4.14新增加进来改进，本来你只能为特定的某个Map配置好GameModeClass，但是现在GameInstance允许你重载它的PreloadContentForURL、CreateGameModeForURL和OverrideGameModeClass方法来hook改变这一流程。

4\. OnlineSession的管理，这部分逻辑跟网络的机制有关（到时候再详细介绍），目前可以简单理解为有一个网络会话的管理辅助控制类。

而GameInstance是在GameEngine里创建的（先不谈UEditorEngine）：

```
void UGameEngine::Init(IEngineLoop* InEngineLoop)
{
    //[...]
    // Create game instance.  For GameEngine, this should be the only GameInstance that ever gets created.
    {
        FStringClassReference GameInstanceClassName = GetDefault<UGameMapsSettings>()->GameInstanceClass;
        UClass* GameInstanceClass = (GameInstanceClassName.IsValid() ? LoadObject<UClass>(NULL, *GameInstanceClassName.ToString()) : UGameInstance::StaticClass());
        if (GameInstanceClass == nullptr)
        {
            UE_LOG(LogEngine, Error, TEXT("Unable to load GameInstance Class '%s'. Falling back to generic UGameInstance."), *GameInstanceClassName.ToString());
            GameInstanceClass = UGameInstance::StaticClass();
        }
        GameInstance = NewObject<UGameInstance>(this, GameInstanceClass);
        GameInstance->InitializeStandalone();
    }
    //[...]
 }
//在BaseEngine.ini或DefaultEngine.init里你可以配置GameInstanceClass
[/Script/EngineSettings.GameMapsSettings]
GameInstanceClass=/Script/Engine.GameInstance
```

先从配置中取出GameInstanceClass，然后动态创建，一目了然。

**思考：GameInstance只有一个吗？**  
一般而言，是的。对于我们自己开发的游戏而言，我们始终只需要关注自己的一亩三分地，那么你可以认为你子类化的那个GameInstance就像个单件一样，全局唯一只有一个，从游戏的开始到结束。但既然是本系列文章的读者，自然也是不甘于只了解这么多的。  
正如把网络连接也当作Player这个概念一样，我们此时也需要重新审视一下Game这个概念。什么是一个Game?对于玩家而言，Game就是从打开到关闭的这整个过程说展现的内容。但是对于开发者来说，这个概念就需要扩充一下了。假设有个引擎支持双击图标一下子开出4个窗口来让4个玩家独立运行，你能说得清这是一个Game还是4个Game在运行吗？哪一种说法都能自圆其说，但关键是哪一种概念划分能更好的让我们管理组织结构。因此针对这种情况，如果是这4个窗口一点都不互相关联，或者只是单独的共用地图资源，那么用4个Game的概念来管理就更为合适。如果这4个窗口里运行的内容，实际上只是在同一个关卡里本地对战，内存里互相直接通信，那用一个Game加上4个Player的概念就会变得更合适。所以针对这点，你可以把Game理解为就像进程一样，进程可以在同一个exe上多开，Game也可以在同一份游戏资源上开出多个运行实例；进程之间可以互相通信协作，Game的不同实例也可以互相沟通，不管是内存中直接在Engine的协调下完成，还是通过Socket通信。  
另一方面，一般游戏引擎都只是服务于游戏本身，而对于其配套的各种编辑器就像是对待外来的打工者一样，编辑器往往只负责最终输出游戏资源。由于应用场景的不同，编辑器的架构也常常根据相应平台而定，五花八门，有用Qt，MFC，WPF等各种平台UI框架。而对于另一些有大志向的引擎，比如Unity和UE，其编辑器就是采用引擎自绘的方案（其优劣暂不分析，以后聊到UI框架再细说）。所以游戏引擎这个时候，就更加的拔高了一个层次，就不再只是个“游戏”引擎了，而是个“程序”引擎了。因此UE本身的这套框架不光要服务游戏，还要服务编辑器，甚至是另外一些辅助程序。所以，Game的概念也就扩充到了更上层的“程序”，变得更广义了。  
言归正传，因为UE的这套Editor自绘机制，还有PIE（PlayInEditor），进程里其实是可以同时有多个GameInstance的，如正在编辑的EditorWorld所属于的，和Play之后的World属于的。我想，这也就是为何UE把它叫做GameInstance而不是简单的Game的含义，其名字中就隐含了多个Instance的深意。我们现在再次回顾一下([GamePlay架构（三）WorldContext，GameInstance，Engine](https://zhuanlan.zhihu.com/p/23167068))最后的结构图，了解一下GameInstance又是被谁管理的：  

![](https://pica.zhimg.com/v2-94d1f4e3750b6f4fd09d02b20bc980b0_1440w.png)

当初我们是以数据的视角，在考察WorldContext的从属的时候讨论过这个结构。现在以逻辑的角度，明白了GameInstance也会被上层的Engine实例出来多个，就会有更深的理解了。

再扩充一下，在Engine之下允许同时运行多个GameInstance，还会有许多其他好处，就像操作系统允许一份资源运行多个进程实例一样，Engine就可以站在更高的层次上管理协调多个Game，同时也能更加的深入到Game内部去得到更多的优化。比如未来要实现游戏本地的host多开并管理，或者在Server同时Host一个Map的多个实例(现在只能一个……还是有很多工作要做啊)，这对于开发MMO网游是非常需要的功能，虽然目前UE在这一块的具体工作还有些薄弱，但至少可扩展的可能性是已经保证了的（动手能力强的高手可以在此基础上定制）。一般而言，间接多一层，就多了一层的灵活性，所以很多引擎其实就是把Game和Engine揉在了一块没有为了GamePlay框架而分开。

**思考：哪些逻辑应该放在GameInstance？**  
第二个惯例的问题是，这一层应该写些什么逻辑。顾名思义，既然是作为游戏中全局唯一的长者，我们就应该给他全局的控制权。在逻辑层面，GameInstance往下看是：  
1\. Worlds，Level的切换实际发生地是Engine，而GameInstance可以说是UE之神其下的唯一代言人，所以GameInstance也可以代之管理World的切换等。我们可以在GameInstance里实现各种逻辑最后调用Engine的OpenLevel等接口。  
2\. Players，虽然一般来说我们直接控制Players的机会不多，都是配置好了就行。但要是到了需要的时候，GameInstance也实现了许多的接口可以让你动态的添加删除Players。  
3\. UI，UE的UI是另一套World之外的系统，虽然同属于Viewport的显示之下，但是控制结构跟Actor们并不一样。所以我们常常会需要控制UI各种切换的业务逻辑，虽然在Widget的Graph里也可以写些简单的切换，但是要想复用某些切换逻辑的时候，在特定的Wdiget里就不合适了，而GameMode一方面局限于Level，另一方面又只存在于Server；PlayerController也是会切换掉的，同时又只存在于World中，所以最后比较合适的就剩下GameInstance了，以后当然有可能了可能会扩展出个UI的业务逻辑Manger类，不过那是后话了。  
4\. 全局的配置，也常常需要根据平台改变一些游戏的配置，Execute一些ConsoleCommand，GameInstance也是这些命令的存放地。  
5\. 游戏的额外第三方逻辑，如果你的游戏需要其他一些控制，比如自己写的网络通信、自定义的配置文件或者自己的一些程序算法，如果简单的话，GameInstance也可以一放，等复杂起来了，也可以把GameInstance当作一个模块容器，你可以在里面再扩展出来其他的子逻辑模块。当然如果是插件的话，还是在自己的插件Module里面自行管理逻辑，然后把协调工作交给GameInstance来做。

而在数据层面上，我们层层上来，已经有了针对一个Player的Contoller的PlayerState，也有了针对World的GameMode的GameState，到了更全局之上，自然的GameInstance就应该存储一些全局的状态数据。所以你可以在GameInstance的成员变量中添加一些全局的状态，或者是那些想要在Level之外持续存在的对象。不过需要注意的一点是，GameInstance成员变量中最好只保存那些“临时”的数据，而对于那些想要持久序列化保存的数据，我们就需要接下来的SaveGame了。把持久的数据直接放在SaveGame，用的时候直接读取出来，之后再直接在其上更新，好处是只用维护一份，省得要保存的时候，还去想到底要选GameInstance的哪些成员变量中来保存，一开始就设计选好，以后就方便了。

## SaveGame

UE连玩家存档都帮你做了！得益于UObject的序列化机制，现在你只需要继承于USaveGame，并添加你想要的那些属性字段，然后这个结构就可以序列化保存下来的。玩家存档也是游戏中一个非常常见的功能，差的引擎一般就只提供给你读写文件的接口，好一点的会继续给你一些序列化机制，而更好的则会服务得更加周到。UE为我们在蓝图里提供了SaveGame的统一接口，让你只用关心想序列化的数据。  
USaveGame其实就是为了提供给UE一个UObject对象，本身并不需要其他额外的控制，所以它的类是如此的简单以至于我能直接把它的全部声明展示出来：

```
UCLASS(abstract, Blueprintable, BlueprintType)
class ENGINE_API USaveGame : public UObject
{
    /**
     *    @see UGameplayStatics::CreateSaveGameObject
     *    @see UGameplayStatics::SaveGameToSlot
     *    @see UGameplayStatics::DoesSaveGameExist
     *    @see UGameplayStatics::LoadGameFromSlot
     *    @see UGameplayStatics::DeleteGameInSlot
     */

    GENERATED_UCLASS_BODY()
};
```

而UGameplayStatics作为暴露给蓝图的接口实现部分，其内部的实现是：  

![](https://pic4.zhimg.com/v2-5f9893415a3b89cb6ef4c2e217cbf391_1440w.png)

先在内存中写入一些SavegameFileVersion之类的控制文件头，然后再序列化USaveGame对象，接着会找到ISaveGameSystem接口，最后交于真正的子类实现文件的保存。目前的默认实现是FGenericSaveGameSystem，其内部也只是转发到直接的文件读写接口上去。但你也可以实现自己的SaveGameSystem，不管是写文件或者是网络传输，保存到不同的地方去。或者是内部调用 [OnlineSubsystem](https://zhida.zhihu.com/search?content_id=1788729&content_type=Article&match_order=1&q=OnlineSubsystem&zhida_source=entity) 的Storage接口，直接把玩家存档保存到Steam云存储中也可以。

因此可见，单单是玩家存档这件边角的小事，UE作为一个深受游戏开发淬炼过的引擎，为了方便自己，也同时造福我们广大开发者，已经实现了这么一套完善的机制。

关于存档数据关联的逻辑，再重复几句，对于那些需要直接在全局处理的数据逻辑，也可以直接在SaveGame中写方法来实现。比如实现AddCoin接口，对外隐藏实现，对内可以自定义附加一些逻辑。USaveGame可以看作是一个全局持久数据的业务逻辑类。跟GameInstance里的数据区分就是，GameInstance里面的是临时的数据，SaveGame里是持久的。清晰这一点区分，到时就不会纠结哪些属性放在哪里，哪些方法实现在哪里了。

注意一下，SaveGameToSlot里的SlotName可以理解为存档的文件名，UserIndex是用来标识是哪个玩家在存档。UserIndex是预留的，在目前的UE实现里并没有用到，只是预留给一些平台提供足够的信息。你也可以利用这个信息来为多个不同玩家生成不同的最后文件名什么的。而ISaveGameSystem是IPlatformFeaturesModule提供的模块接口，关于模块的机制，等引擎流程章节再说吧，目前可以简单理解为一个单件对象里提供了一些平台相关的接口对象。

## 总结

至此，我们可以说已经介绍完了GamePlay下半部分——逻辑控制。在蓝图层，UE并不向BP直接暴露Engine概念，即使在C++层，在实现GamePlay业务时也是很少需要真正直接操纵Engine的时候。如果GamePlay已经足够好，那么Engine自然就可以隐居幕后了。UE用GameInstance实现了全局的控制，并支持多GameInstance来实现编辑器，最后在存档的时候还可以用到SaveGame的方便的接口。  
下篇，就是GamePlay章节的最终章，我们将会对GamePlay架构的（一到九）篇进行回顾归纳总结巩固，以一个承上启下总览的眼光，再来重新审视一下UE的整套GamePlay框架，下个章节见。

上篇： [《InsideUE4》GamePlay架构（八）Player](https://zhuanlan.zhihu.com/p/23826859)

下篇： [《InsideUE4》GamePlay架构（十）总结](https://zhuanlan.zhihu.com/p/24170697)

## 引用

1. [SaveGame](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/latest/INT/Gameplay/SaveGame/index.html)

*UE4.14*

\---------------------------------------------------------------------------------------------------------------------------

知乎专栏： [InsideUE4](https://zhuanlan.zhihu.com/insideue4)

UE4深入学习QQ群： **456247757** (非新手入门群，请先学习完官方文档和视频教程)

微信公众号： **aboutue** ，关于UE的一切新闻资讯、技巧问答、文章发布，欢迎关注。

**个人原创，未经授权，谢绝转载！**

编辑于 2019-01-01 17:33[游戏引擎](https://www.zhihu.com/topic/19556258)[虚幻引擎](https://www.zhihu.com/topic/19824201)[游戏开发](https://www.zhihu.com/topic/19553361)