---
title: "《InsideUE4》GamePlay架构（十一）Subsystems"
source: "https://zhuanlan.zhihu.com/p/158717151"
author:
  - "[[大钊UE源码剖析，等编译的时候回答问题]]"
published:
created: 2026-04-18
description: "大家好，我厚着脸皮又回来了引言非常惭愧，自从我更新完GamePlay架构十篇之后，我就断更了许久。如今说再多缘由也是借口，借着假期，在家继续重操旧业，继续写写技术文章。 UE在4.22版本的时候，开始引入Subsystem…"
tags:
  - "clippings"
---
> 大家好，我厚着脸皮又回来了

## 引言

非常惭愧，自从我更新完GamePlay架构十篇之后，我就断更了许久。如今说再多缘由也是借口，借着假期，在家继续重操旧业，继续写写技术文章。

UE在4.22版本的时候，开始引入Subsystems，然后在4.24完善。写本篇文章的一个原因是Subsystems其实可以算是GamePlay框架基础上的一个增强功能，属于GamePlay架构的范围，因此我要补完。另一个原因是我观察到大家对Subsystems好像还很陌生，讨论和用的人都很少。这着实有点可惜，Subsystems其实是一个非常便利的功能，如果大家在编写代码的时候，能够 **有意识的** 利用好Subsystems这个特性的话，会使自己的程序架构更加的清晰和便利。

希望本文能帮助到你达成这一点。为了好好阐述好这个问题，本文的篇幅会比较长，大家可以慢慢挑自己感兴趣的部分，不用着急。

为避免接下来混淆，本文先定义一下接下来使用的术语：

- **Subsystems** ：指的是这整套“子系统”框架，包含了定义的类以及运作机制。
- **[SubsystemType](https://zhida.zhihu.com/search?content_id=123403829&content_type=Article&match_order=1&q=SubsystemType&zhida_source=entity) /SubsystemClass** ：指向的是Subsystem的类型，比如TSubclassOf<USubsystem>。
- **Subsystem对象** ：指的是真正创建生成实例化出来的Subsystem对象。
- **UMyXXXSubsystem**: 用户定义的类，我会以My为前缀来区分。
- **5类Outer对象** ：Subsystem对象依存属于的5类Outer对象。
1. UEngine\* GEngine;
2. UEditorEngine\* GEditor;
3. UGameInstance\* GameInstance;
4. UWorld\* World;
5. ULocalPlayer\* LocalPlayer;
- **生命周期5类** ：引擎已经预定义的5个让你可以由此派生的父类。
1. UEngineSubsystem
2. UEditorSubsystem
3. UGameInstanceSubsystem
4. UWorldSubsystem
5. ULocalPlayerSubsystem

## Subsystems是什么？

一句话： **Subsystems是一套可以定义自动实例化和释放的类的框架。** 这个框架允许你从5类里选择一个来定义子类(只能在C++定义)：

![](https://pic1.zhimg.com/v2-1a596ce3195ccaf4275b0aa88dedf33e_1440w.jpg)

之后就可以享受以下这些福利了：

1. **自动实例化**

这些的UMyXXXSubsystem类，会在合适的时机被创建出对象，然后在合适的时机释放，这个过程是自动化的。不需要自己手写创建代码。也不需要自己显式的定义变量，Subsystems已经定义好方便友好的访问接口了。

**2\. 托管生命周期**

根据你选择的父类不同，引擎会为创建出来的Subsystem实现出不同的生命周期。因此官方文档里会称这5个父类为5个不同的生命周期。根据你选择的生命周期不同，Initialize()和Deinitialize()会自动的在合适的时机被调用。一个Subystem类型也有可能根据需要被自动的被创建出多个实例。这些里面的繁琐逻辑自己都不用操心。

## Subsystems的基本使用

在谈到为什么需要Subsystems，以及如何用好Subsystems之前，我们先了解一些Subsystems的基础使用知识。先懂怎么用，然后再谈为什么，以及怎么用好。 目前Subsystems的使用只能在C++端，用起来倒也还简单。

**第一步，定义C++子类：**

```cpp
//声明定义：
UCLASS() 
class HELLO_API UMyEditorSubsystem : public UEditorSubsystem
UCLASS() 
class HELLO_API UMyEngineSubsystem : public UEngineSubsystem
UCLASS() 
class HELLO_API UMyGameInstanceSubsystem : public UGameInstanceSubsystem
UCLASS() 
class HELLO_API UMyWorldSubsystem : public UWorldSubsystem
UCLASS() 
class HELLO_API UMyLocalPlayerSubsystem : public ULocalPlayerSubsystem

//注：使用UEditorSubsystem需要在Build.cs里加上EditorSubsystem模块的引用，因为这是编辑器模块
if (Target.bBuildEditor)
{
    PublicDependencyModuleNames.AddRange(new string[] { "EditorSubsystem" });
}
```

**第二步，像普通的UObject类一样，可以在里面定义属性和函数。**

以一个非常简单的分数系统为例：

```cpp
UCLASS()
class HELLO_API UMyScoreSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public://重载的函数，可以做一些初始化和释放操作
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
    virtual void Initialize(FSubsystemCollectionBase& Collection)override;
    virtual void Deinitialize()override;
public:
    UFUNCTION(BlueprintCallable)
    void AddScore(float delta);
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Score;
};
```

**第三步，就可以在C++和蓝图里访问这些类和调用函数了。**

**C++里的访问：**

```cpp
//UMyEngineSubsystem获取
UMyEngineSubsystem* MySubsystem = GEngine->GetEngineSubsystem<UMyEngineSubsystem>();

//UMyEditorSubsystem的获取
UMyEditorSubsystem* MySubsystem = GEditor->GetEditorSubsystem<UMyEditorSubsystem>();

//UMyGameInstanceSubsystem的获取
UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(...);
UMyGameInstanceSubsystem* MySubsystem = GameInstance->GetSubsystem<UMyGameInstanceSubsystem>();

//UMyWorldSubsystem的获取
UWorld* World=MyActor->GetWorld();  //world用各种方式也都可以
UMyWorldSubsystem* MySubsystem=World->GetSubsystem<UMyWorldSubsystem>();

//UMyLocalPlayerSubsystem的获取
ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerController->Player)
UMyLocalPlayerSubsystem * MySubsystem = LocalPlayer->GetSubsystem<UMyLocalPlayerSubsystem>();
```

当然引擎源码里也已经定义好了一些方便的蓝图库函数，USubsystemBlueprintLibrary里的函数虽然不暴露在蓝图端，但也是可以在C++端方便调用的。

```cpp
//我省略了一些宏标记和注释，因为函数名字是不言自明的。
UCLASS()
class ENGINE_API USubsystemBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    static UEngineSubsystem* GetEngineSubsystem(TSubclassOf<UEngineSubsystem> Class);
    static UGameInstanceSubsystem* GetGameInstanceSubsystem(UObject* ContextObject, TSubclassOf<UGameInstanceSubsystem> Class);
    static ULocalPlayerSubsystem* GetLocalPlayerSubsystem(UObject* ContextObject, TSubclassOf<ULocalPlayerSubsystem> Class);
    static UWorldSubsystem* GetWorldSubsystem(UObject* ContextObject, TSubclassOf<UWorldSubsystem> Class);
    static ULocalPlayerSubsystem* GetLocalPlayerSubSystemFromPlayerController(APlayerController* PlayerController, TSubclassOf<ULocalPlayerSubsystem> Class);
};
```

蓝图内的访问：

![](https://pic3.zhimg.com/v2-0fa07d1fc3f269b31c723f5b345ae6ce_1440w.jpg)

用起来的感觉有点像“全局变量”，也有点像是静态蓝图函数（如GetGameInstance），可以方便的在蓝图的各个地方调用。但Subsystems真正的威力其实远不止这点手头上的便利，而在于接下来要谈的引擎帮你自动处理的部分。

## 如何理解Subsystems的生命周期？

可以说Subsystems机制的核心之处就是在于引擎帮你托管了对象的生命周期，而理解这些对象生命周期的不同，也是理解Subsystems的关键之处。

**简短的说，Subsystem对象的生命周期取决于其依存的Outer对象的生命周期，随着Outer对象的创建而创建，随着Outer对象的销毁而销毁。而选择依存哪种Outer对象，就是选择哪种Subsystem生命周期，靠的就是选择继承于哪个Subsystem基类。**

从数据角度分析，先一图以概之：

![](https://picx.zhimg.com/v2-c4e2d200db7386c086cb59ab49a48449_1440w.jpg)

从源码里分析，像5类这种对象，比如GEngine，里面都添加了一个FSubsystemCollection<TBaseType> SubsystemCollection的对象，而其基类FSubsystemCollectionBase里面存储了Subsystem对象的引用。

```cpp
class ENGINE_API UEngine
{
private:
    FSubsystemCollection<UEngineSubsystem> EngineSubsystemCollection;
};
//而FSubsystemCollection又继承于FSubsystemCollectionBase
template<typename TBaseType>
class FSubsystemCollection : public FSubsystemCollectionBase
{
};
//而FSubsystemCollectionBase里面用Map存储了对象引用
class ENGINE_API FSubsystemCollectionBase : public FGCObject
{
private:
    TMap<TSubclassOf<USubsystem>, USubsystem*> SubsystemMap;//用Map来存储对象
    TSubclassOf<USubsystem> BaseType;//Subsystem对象类型
    UObject* Outer;//外部的对象
}
```

可以看到，这5类Outer对象(UEngine,UEditorEngine,UGameInstance,UWorld,ULocalPlayer)里面都新增了一个FSubsystemCollection<TBaseType> SubsystemCollection的成员变量，用来存储其关联的Subsystem对象。 拿最常用的UGameInstance来举个例子，假如你定义了两个自己UScoreSubsystem(计分系统)和UTaskSubsystem(任务系统)，全都继承于UGameInstanceSubsystem之后，你的对象布局应该是这个样子：

![](https://pic2.zhimg.com/v2-969907e096b86475d17269a8d75e6d79_1440w.jpg)

可以看到GameInstance里的FSubsystemCollection对象存储了生成的UScoreSubsystem和UTaskSubsystem对象的引用，而这二者其Outer都是指向GameInstance对象。数据内存结构还是比较简单的，但也有一些要点：

1. 我特意标明了FSubsystemCollectionBase继承于FGCObject，意在说明虽然FSubsystemCollection是个结构，其在UGameInstance里，但其内部的对象引用也是受到GC管理的。FGCObject的内容不是此文重点，因此不再赘述，只要明白它是个让F结构也可以让GC管理内部U对象引用的机制。
2. FSubsystemCollectionBase里的UObject\* Outer，指向外部的UGameInstance对象。这个Outer可以用来在USubsystem::ShouldCreateSubsystem()或FSubsystemCollectionBase::Initialize(FSubsystemCollectionBase& Collection)的时候，从而可以在某个USubsystem对象创建之前获取到外部Outer，从而继续获取到其他的兄弟姐妹Subsystem对象，从而做一些逻辑判断。当然创建完之后，因为USubsystem对象的Outer其实也为UGameInstance，所以直接GetOuter()也就可以了。
3. 从TMap的Key为TSubclassOf可以看出， **一种特定类型的USubsystem子类只能创建出一个USubsystem对象** 。所以UScoreSubsystem和UTaskSubsystem可以同时存在，但一种也只能有一个，类似单件模式。
4. 如果查看UGameInstanceSubsystem的源码（其他同理）：
```cpp
UCLASS(Abstract, Within = GameInstance)
class ENGINE_API UGameInstanceSubsystem : public USubsystem
{
}
```

解释一下两个重要的宏标记：

- Abstract标记指明UGameInstanceSubsystem是抽象基类，是不能被直接创建出来的。
- Within = GameInstance，Within这个标记的意思是其对象的Outer必须是某个类型，另外Within的标记是会被继承到子类的。所以综合的意思就是继承于UGameInstanceSubsystem的之类Subsystem对象的Outer必须是GameInstance，保证了其对象的依存合法性。所以其实你也是不能自己随便NewObject()出来的，避免了自己误操作。

总结一下，在Subsystems之前我们其实也可以自己用C++来实现一个类似的“单例模式”，也可以达成类似的效果，其实大家也往往就这么干。但Subsystems给我们带来的远不止这些，因此接下来我们就来谈谈Subsystems相比我们自己手写的优点有哪些。

## 为什么要引进Subsystems？

首先，我非常赞同官方引入Subsystems这套机制，虽然其实工作量不多，实际源码行数也很少，但是这有效的弥补了程序实践上的一个易混乱缺陷漏洞。为了让你们能真正学习Subsystems并开始用起来，下面我就来好好安利一番：

**一， 不用自己手写，懒人福音，而且更正确**

减少Bug最好的方式就是少写代码。诚然Subsystems这套并不是多高深的技术，实现的原理也蛮简单，差不多的程序员都能自己实现一套。但是引擎是否内建提供了该通用机制依然很重要。举例来说，你如果也想实现个类似的单件模式，你可能需要这么写：

```cpp
UCLASS()
class HELLO_API UMyScoreManager : public UObject
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintPure,DisplayName="MyScoreManager")
    static UMyScoreManager* Instance()
    {
        static UMyScoreManager* instance=nullptr;
        if (instance==nullptr)
        {
            instance=NewObject<UMyScoreManager>();
            instance->AddToRoot();
        }
        return instance;
        //return GetMutableDefault<UMyScoreManager>();
    }
public:
    UFUNCTION(BlueprintCallable)
        void AddScore(float delta);
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Score;
};
```

这段代码有什么问题？

1. 学习负担重，新人看不明白。首先第一个问题就是，但凡是想要用单件模式的人，都得先自己学会这么一编写套路，而且得抄对。
2. AddRoot()往往很多人会忘掉，导致对象被释放，自己却拿它来调用函数是，导致崩溃。
3. 这里用GetMutableDefault可不可以？直接采用CDO对象当做单件对象可不可以，区别是什么？又有多少人能分得清呢。实际上二者确实差别不大，只不过是一个是否保留默认值的问题，常常确实可以通用。但GetMutableDefault其实我发现知道和用的人不多。Subsystem是没有直接用CDO的，因为其可能会被反复的创建销毁，所以需要CDO来恢复默认值。
4. 这个单件的存续期其实在Editor模式下也会存在，所以在Play和Stop后其值依然会存在，表现为脏值。这也是一个常见的单件陷阱问题。
5. 因此是否考虑重复创建和销毁的时机？像上述这种写法，单件创建出来后就一直存在。这个时候想处理这个创建销毁的问题又有几个选择：一是再加上Initialize和Deinitialize()函数来手动调用；二是换种写法，不用函数静态变量，而用类静态变量再加上Destroy方法手动调用；三是把static UMyScoreManager\* 放到UGameInstance的子类里当做类静态变量，然后在Init和Shutdown的重载函数里进行创建和释放。不同的写法也都有其优缺点，但最大的缺点是： **你要为每个单件类都写一遍，而且不能写错。**
6. 维护成本高。手动模式要求你为每个新单件类都要记得加上其创建和释放的代码操作。根据第5点，当新定一个MyNewManager，你就得记得先定义静态变量，然后Init里加上创建代码，在ShutDown里加上销毁代码。而引擎的这套机制可以利用反射机制，方便的创建和销毁对象。可维护性比手写的要强多了。
7. 当然也有人会说用Engine内建提供好的GameInstanceClass，但是缺点是只支持一个类型，而且生命周期是整个引擎，不是一场游戏。
![](https://pic1.zhimg.com/v2-be0dd217ee405221eeddc89d7f9281a6_1440w.jpg)

总结一下，你或许可以自己手写出一套足够健壮灵活的机制，但想做好做正确其实并不简单。而且因为你不能侵入引擎源码(受引擎分发升级等限制)，所以你自己实现的方式总会受限。最好的方式是引擎实现好了之后，你理解并用好它。

**二、 更模块化**

很多人，包括我自己，在实现一些全局的游戏系统的时候，总是从UGameInstance继承下来，然后在里面码一堆代码：

```cpp
UCLASS()
class HELLO_API UMyGameInstance : public UGameInstance
{
    GENERATED_BODY()
public://UI系统
    UFUNCTION(BlueprintCallable)
        void PushUI(TSubclassOf<UUserWidget> widgetClass);
public://任务系统
    UFUNCTION(BlueprintCallable)
        void CompleteTask(FString taskName);
public://计分系统
    UFUNCTION(BlueprintCallable)
        void AddScore(float delta);
};
```

示例代码我就意思一下，但请读者们自己回想一下，自己是否也曾经这么干过呢？有的人请在公屏上打上“确实”。但这么写其实也有三个大问题：

**1\. 一个类里挤占着太多逻辑。** 可维护性差，不利于分工协作。一个游戏常常有很多个逻辑模块，如任务系统，计分系统，经济系统、对话系统、爆率系统等等，这些都实现在UMyGameInstance里，会导致类越来越庞大，逐渐超过自己的心智负担上限。

**2\. 不利于模块复用。** 假如你下载了个游戏模板，里面有个对话系统，你觉得很好想挪用过来。但人家也是写在UDialogueGameInstance里的，这个时候你只能手动把其代码拷贝到你的UMyGameInstance里。或者说，你的另一款游戏也想复用上款游戏的一些逻辑模块，于是你也只能手动拷贝一遍。想想之前是不是经常这么干？

**3\. 手动划分Manager类也不够机智。** 当然一些机智的人已经想到了可以把这些业务逻辑模块，划分为不同的Manager类。但问题依旧，你依然需要自己手动去管理这些Manager类的生命周期。

因此，现在有了Subsystems之后，你只需要把这些都写成:

```cpp
UCLASS()
class HELLO_API UMyUISubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public://UI系统
    UFUNCTION(BlueprintCallable)
        void PushUI(TSubclassOf<UUserWidget> widgetClass);
};

UCLASS()
class HELLO_API UMyTaskSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public://任务系统
    UFUNCTION(BlueprintCallable)
        void CompleteTask(FString taskName);
};

UCLASS()
class HELLO_API UMyScoreSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public://计分系统
    UFUNCTION(BlueprintCallable)
        void AddScore(float delta);
};
```

这么写有几点好处：

**1\. 代码看起来优雅多了。** 相应的逻辑组织在对应的类里，易于理解，易于维护，也易于分工协作（可以一个人写一个模块）。

**2\. 自己不用手写Subsystem对象的创建释放逻辑了。** 解放了双手，也免于出错。

**3\. 容易迁移复用。** 想复用一个之前已经写好的比如任务系统，只需要把那个类直接拷贝过来就行了。不需要进行代码的手动拷贝粘贴工作，避免出错。

**4\. 更好的封装粒度，避免系统之间的数据访问污染。** 按之前的方式，在UMyGameInstance里面，你在实现任务系统的时候，比如StartTask，你可能还需要用到计分系统的Score变量，这个时候你往往可能就直接访问成员变量了。因此， **多个逻辑模块之间的数据其实是互相污染的** ，这是一种耦合紧密，封装不够良好的坏味道。如今拆成多个Subsystem类之后，就可以在特定的系统里，如UMyScoreSubsystem::GetScore()里面做更好的封装和验证。

**三，理解的一致性**

当程序员之间进行沟通程序架构的时候，设计模式是一些约定俗成的招数。同样，在UE4里进行逻辑复用的时候，也需要更加的清晰的统一认识。 让我们设想这么一个场景，你从虚幻商城里下载了一个游戏模板，比如一套战斗系统模板。你在去理解怎么把它复用到你的游戏项目里的时候，你会按照什么思路去理解它？你首先要看的是什么文件？如果作者没有写清楚说明文档的话，我发现往往也都是先从GamePlay那几个类先看看里面有什么，然后去Demo关卡里看看用到了哪些Actor类，接着运行起来看看，然后各个目录翻翻。这里面的问题在于， **不同的开发者在编程的时候都有自己的方式，互相之前缺少共识。**

在有了Subsystem之后，如果大家都学习和利用好Subsystem，把自己的功能模块用Subsystem来实现。这样大家都看和理解别人的模块的时候，首先想到的就是去看看对方里面有啥Subsystem类，然后专注的看它就行了。要复用的话也是直接拷贝过来就行。

总结一下，UE里如果提到复用的理解一致性：

- **通用功能的复用** ：从各个ActorComponent里查看，这一层代表的往往是跟“游戏逻辑”无关的可复用功能。
- **业务逻辑的复用** ：从Subsystem来查找，其代表就是游戏逻辑相关的可复用部分。

至于GamePlay里的各个类，那其实是大家对于业务框架流程的一致性理解，往往并不能直接复用拷贝过来使用。

**四，避免重载引擎类**

特别是在实现插件模块的时候，其实你并不需要继承于GameInstance或之类的，但是往往你又需要一个“全局的管理类”，于是不得不为之。继承于GameInstance的坏处上文已经说过了，有些人有鉴于此，往往就另建了一个“XXXManagerActor”蓝图类，然后要求用户拖放这个Actor到场景内来使它工作。这其实也只是一种“绕开”的办法而已，从结构上来说不够优雅，从用户使用体验上来说也不够直接。

**五，生命周期控制粒度更细**

如果说想让自己的Manager类依存UGameInstance的生命周期还好实现，因为UGameInstance里已经提供了Init和Shutdown的重载，但如果想实现依存Engine，Editor，World，LocalPlayer的不同生命周期，就难很多了。你得去注册这些对象的创建销毁事件，如果没有提供的话，就只能去改源码了，必须修改引擎来在合适的时机插入初始化删除代码。因此相比，Subsystem的实现由引擎开发人员已经实现好了，我们不用自己处理这点脏活，省心。

**六，更友好的访问接口**

虽然说UE已经提供了Subsystem的Python的访问，但我在UE里用Python不多，因此不是特别在意。光从蓝图的访问接口来说，Subsystem的样子更像个全局变量，而用静态函数模拟的就还只是个函数。这点差别对强迫症来说 算是一点小福利吧。

![](https://pic2.zhimg.com/v2-332b42cd92e52826a2c3a2bd85af69a9_1440w.jpg)

但其实还有一点比较隐晦的是，Subsystem有个“上下文”的概念。相比MyScoreManager这种全局函数在哪里都可以访问来获得值，Subsystem的访问接口里会判断当前所属于的对象ContextObject是否能够获得相应的Subsystem对象。比如UWorldSubsystem，就必须在可以根据当前蓝图对象得到World的时候才能访问值，否则是null。比如你如果在一个UserWidget里，或者在一个UObject子类对象里，虽然可以写下MyWorldSubsystem节点，但是返回的会是null。ULocalPlayersystem也是同理。关于“上下文”对象的理解，我们在后面再详述。

## 能否再具体解释一下Subsystem对象的创建销毁流程？

如果你被我成功安利到了，想要开始使用Subsystems，那下一个问题就是如何用好Subsystems呢？我一直有一个观点，纯熟的应用是要建立在深刻的理解之上的。因此且让我们再花一些时间阐述一下Subsystem对象的具体创建的流程。做到知其然且知其所以然，心中不慌。 从源码上进行分析，虽然真正核心流程并不长，但具体叙述起来也是篇幅太长，因此这里我叙述一些要点：

**一，深刻理解5类Outer对象的生命周期。**

理解这些引擎重要的内建类的生命周期机制是理解的基础。因此也请读者事先知道引擎编辑器模式、Runtime模式和PIE(Play In Editor)模式这三个的区别。编辑器模式就是你启动编辑器，Runtime模式是指你游戏打包后的运行状态，PIE模式指的是在编辑器里点击Play后的运行状态。

1. **UEngine\* GEngine： 代表引擎，数量1。** Editor或Runtime模式都是全局唯一，从进程启动开始创建，进程退出时销毁。
2. **UEditorEngine\* GEditor： 代表编辑器，数量1** 。 顾名思义，只在编辑器下存在且全局唯一，从编辑器启动开始创建，到编辑器退出时销毁。
3. **UGameInstance\* GameInstance：代表一场游戏，数量1。** 从游戏的启动开始创建，游戏退出时销毁。这里的一场游戏指的是Runtime或PIE模式的运行的都算，一场游戏里可能会创建多个World切换。
4. **UWorld\* World：代表一个世界，数量可能>1。** 对此不明白的建议读者去看我之前的GamePlay架构文章。简单来说，World和GameMode是关联的，可以包含多个Level，默认情况下OpenLevel常常会切换World。因此其生命周期，其实跟GameMode是一起的。但也请读者建议，编辑器模式下视口里的场景其实也是个World，因此EWorldType其实有多个类型：Game，Editor，PIE，EditorPreview，GamePreview等。读者需知道，编辑器下的视口场景也是个World!
5. **ULocalPlayer\* LocalPlayer：代表本地玩家，数量可能>1。** UE支持本地分屏多玩家类型的游戏，但往往最常见的是就只有一个。LocalPlayer虽然往往跟PlayerController一起访问，但是其生命周期其实是跟UGameInstance一起的(默认一开始的时候就创建好一定数量的本地玩家)，或者更准确的说是跟LocalPlayer的具体数量挂钩（当然你也可以运行时动态调用AddLocalPlayer)。

因此你想理解好那5个Subsystem类的生命周期，就得理解这5个Host对象的生命周期。道理很简单，Subsystem对象的创建和销毁都是在Host对象的创建和销毁的时候相应执行的。

**二，理解Subsystem对象反射创建销毁流程**

这5类Outer对象自己创建的时候，都会调用一下 `FSubsystemCollectionBase::Initialize(UObject* NewOuter)` ，把自己this作为Outer传进去。因此其Subsystem对象的创建流程其实就在这个函数里面：

```cpp
void FSubsystemCollectionBase::Initialize(UObject* NewOuter)
{
    if (BaseType->IsChildOf(UDynamicSubsystem::StaticClass()))//如果是UDynamicSubsystem的子类
    {
        for (const TPair<FName, TArray<TSubclassOf<UDynamicSubsystem>>>& SubsystemClasses : DynamicSystemModuleMap)
        {
            for (const TSubclassOf<UDynamicSubsystem>& SubsystemClass : SubsystemClasses.Value)
            {
                if (SubsystemClass->IsChildOf(BaseType))
                {
                    AddAndInitializeSubsystem(SubsystemClass);
                }
            }
        }
    }
    else
    {   //普通Subsystem对象的创建
        TArray<UClass*> SubsystemClasses;
        GetDerivedClasses(BaseType, SubsystemClasses, true);//反射获得所有子类

        for (UClass* SubsystemClass : SubsystemClasses)
        {
            AddAndInitializeSubsystem(SubsystemClass);//添加初始化Subsystem对象创建
        }
    }
}
```

UDynamicSubsystem的部分，我们稍后再讲。先来讲那些从USubsystem直接继承下来的子类对象创建，可以看到第一步是由反射获得BaseType（就是生命周期5类）的所有Subsystem子类。然后对其一一AddAndInitializeSubsystem：

```cpp
bool FSubsystemCollectionBase::AddAndInitializeSubsystem(UClass* SubsystemClass)
{
    //...省略一些判断语句，咱们只看核心代码
    const USubsystem* CDO = SubsystemClass->GetDefaultObject<USubsystem>();
    if (CDO->ShouldCreateSubsystem(Outer))  //从CDO调用ShouldCreateSubsystem来判断是否要创建
    {
        USubsystem*& Subsystem = SubsystemMap.Add(SubsystemClass);//创建且添加到Map里
        Subsystem = NewObject<USubsystem>(Outer, SubsystemClass);//创建对象

        Subsystem->InternalOwningSubsystem = this;//保存父指针
        Subsystem->Initialize(*this);   //调用Initialize

        return true;
    }
}
```

这一段代码就核心说明了ShouldCreateSubsystem和Initialize的作用！因此自己可不要忘了重载哦~

**那销毁呢？** 其实在玩家退出游戏或者按照生命周期，遇到Outer对象要被销毁的时候，其内部都加了一句： `SubsystemCollection.Deinitialize();`，依旧来欣赏一下代码：

```cpp
void FSubsystemCollectionBase::Deinitialize()
{
    //...省略一些清除代码
    for (auto Iter = SubsystemMap.CreateIterator(); Iter; ++Iter)   //遍历Map
    {
        UClass* KeyClass = Iter.Key();
        USubsystem* Subsystem = Iter.Value();
        if (Subsystem->GetClass() == KeyClass)
        {
            Subsystem->Deinitialize();  //反初始化
            Subsystem->InternalOwningSubsystem = nullptr;
        }
    }
    SubsystemMap.Empty();
    Outer = nullptr;
}
```

逻辑也非常简单，遍历然后Deinitialize就是！

### 思考：那Subsystem是怎么被GC掉的呢？

我们在上述的代码里并不会看到手动的调用Destroy，因为USubsystem对象是个UObject对象，其依然是受GC管理的。Subsystem对象和Outer对象之前隔了一个FSubsystemCollection结构，为了让F结构依然可以追溯到对象引用，因此FSubsystemCollectionBase继承于FGCObject，所以我们也能找到如下代码：

```cpp
void FSubsystemCollectionBase::AddReferencedObjects(FReferenceCollector& Collector)
{
    Collector.AddReferencedObjects(SubsystemMap);
}
```

当 `FSubsystemCollectionBase::Deinitialize()` 里进行 `SubsystemMap.Empty();`后，USubsystem对象就没有被持有引用了，在下一帧的GC的时候，就会被判定为PendingKill的对象，从而得到Destroy。 这里的妙处是，直接利用了UObject对象之间引用所带来的生命周期绑定机制，来直接把USubsystem对象的生命周期和其Outer对象关联起来，而不用写重复的代码。秒啊！

**三，理解UDynamicSubsystem**

从一开始的类继承结构来看，读者们肯定已经发现UEngineSubsystem和UEditorSubsystem是继承于UDynamicSubsystem的，那为什么要如此设计呢？

顾名思义，其为动态Subsystem，动态二字，表明其会被动态的对待。 **这里的动态特指随着模块的加载释放来创建和销毁。** 要理解这点，首先要理解UE4的模块机制，简单来说，一个uproject项目或uplugin插件可以包含多个Module模块，每个Module可以有个Build.cs，每个模块可以被编译成一个Dll。模块之间可以互相引用。因此一个模块可能有多个依赖的其他模块，我们假如叫它：DependencyModules。引擎的机制是加载一个模块的时候会自动的加载DependencyModules，

首先假设我有一个插件叫做MyPlugin.uplugin，然后有个项目Hello.uproject，Hello项目想使用MyPlugin插件。MyPlugin里依次都定义了：UMyPluginEngineSubsystem，UMyPluginEditorSubsystem和UMyPluginGameInstanceSubsystem。

**显式添加依赖的方式：事先配置好DependencyModules**

当你在Hello项目插件设置上上开启MyPlugin，或者在Hello.uproject里写上 `"Plugins": [{"Name": "MyPlugin","Enabled": true}]` ，或者在Build.cs里加上 `PublicDependencyModuleNames.Add("MyOtherModule");`，这些方式其实都是在显式的在一开始配置好项目的DependencyModules。这样当Hello模块启动的时候，引擎会自动的加载其依赖模块(MyPlugin)，从而你会发现UMyPluginEngineSubsystem和UMyPluginEditorSubsystem在编辑器一启动的时候就会创建并Initialize。 但是如下的代码分支，其实是只在GEditor初始化的时候才会调用到，因为这个时候其他插件里模块还没有加载。

```cpp
void FSubsystemCollectionBase::Initialize(UObject* NewOuter)
{
    if (BaseType->IsChildOf(UDynamicSubsystem::StaticClass()))//如果是UDynamicSubsystem的子类
    {
        for (const TPair<FName, TArray<TSubclassOf<UDynamicSubsystem>>>& SubsystemClasses : DynamicSystemModuleMap)
        {
            for (const TSubclassOf<UDynamicSubsystem>& SubsystemClass : SubsystemClasses.Value)
            {
                if (SubsystemClass->IsChildOf(BaseType))
                {
                    AddAndInitializeSubsystem(SubsystemClass);
                }
            }
        }
    }
    else
    {   //普通Subsystem对象的创建
    }
}
```

这个时候加载的DynamicSystemModuleMap中最重要的就是UnrealEd模块。UnrealEd里面本身其实也已经定义了几个Subsystem: AssetEditorSubsystem，BrushEditingSubsystem，ImportSubsystem，LayersSubsystem和PanelExtensionSubsystem。

**那MyPlugin里的Subsystem什么时候创建呢？**

还是这块代码，其实在第一次初始化的时候，就用FSubsystemModuleWatcher

```cpp
void FSubsystemCollectionBase::Initialize(UObject* NewOuter)
{
    if (SubsystemCollections.Num() == 0)//静态变量，用数目来判断是第一次创建
    {
        FSubsystemModuleWatcher::InitializeModuleWatcher();
    }
}

void FSubsystemModuleWatcher::InitializeModuleWatcher()
{
    // 获得所有UDynamicSubsystem的子类
    TArray<UClass*> SubsystemClasses;
    GetDerivedClasses(UDynamicSubsystem::StaticClass(), SubsystemClasses, true);

    for (UClass* SubsystemClass : SubsystemClasses) //遍历
    {
        if (!SubsystemClass->HasAllClassFlags(CLASS_Abstract))  //不为抽象类
        {
            UPackage* const ClassPackage = SubsystemClass->GetOuterUPackage();//获得所属于的包
            if (ClassPackage)
            {
                const FName ModuleName = FPackageName::GetShortFName(ClassPackage->GetFName());
                if (FModuleManager::Get().IsModuleLoaded(ModuleName))
                {
                    TArray<TSubclassOf<UDynamicSubsystem>>& ModuleSubsystemClasses = FSubsystemCollectionBase::DynamicSystemModuleMap.FindOrAdd(ModuleName);
                    ModuleSubsystemClasses.Add(SubsystemClass);//添加到DynamicSystemModuleMap
                }
            }
        }
    }
    //注册模块加载和释放事件
    ModulesChangedHandle = FModuleManager::Get().OnModulesChanged().AddStatic(&FSubsystemModuleWatcher::OnModulesChanged);
}
```

其中最重要的就是遍历当前进程里的UDynamicSubsystem子类们，并按照模块划分存储进DynamicSystemModuleMap，这样之后就知道当加载或释放某个模块的时候，应该去创建和销毁哪些Subsystem类型对象。

第二步是OnModulesChanged事件的注册，这样在后面加载的模块就能够得到通知：

```cpp
void FSubsystemModuleWatcher::OnModulesChanged(FName ModuleThatChanged, EModuleChangeReason ReasonForChange)
{
    switch (ReasonForChange)
    {
    case EModuleChangeReason::ModuleLoaded:
        AddClassesForModule(ModuleThatChanged);//创建一个模块的DynamicSubsystem类们
        break;

    case EModuleChangeReason::ModuleUnloaded:
        RemoveClassesForModule(ModuleThatChanged);//销毁一个模块的DynamicSubsystem类们
        break;
    }
}
```

而这两个方法的实现其实也挺简单，就是查找该代码模块里定义的类看看是否是UDynamicSubsystem子类，然后为其创建对象实例。

```cpp
void FSubsystemModuleWatcher::AddClassesForModule(const FName& InModuleName)
{
    // 找到这个模块所定义的代码包
    const UPackage* const ClassPackage = FindPackage(nullptr, *(FString("/Script/") + InModuleName.ToString()));

    TArray<TSubclassOf<UDynamicSubsystem>> SubsystemClasses;
    TArray<UObject*> PackageObjects;
    GetObjectsWithOuter(ClassPackage, PackageObjects, false);//得到该模块里定义的所有对象
    for (UObject* Object : PackageObjects)
    {
        UClass* const CurrentClass = Cast<UClass>(Object);//转成UClass试试
        if (CurrentClass && !CurrentClass->HasAllClassFlags(CLASS_Abstract) && CurrentClass->IsChildOf(UDynamicSubsystem::StaticClass()))//满足这些条件
        {
            SubsystemClasses.Add(CurrentClass);
            FSubsystemCollectionBase::AddAllInstances(CurrentClass);//为这个类创建对象实例
        }
    }
    if (SubsystemClasses.Num() > 0)//如果其内部有定义Subsystem类，有些可能没有
    {   //就需要登记一下
        FSubsystemCollectionBase::DynamicSystemModuleMap.Add(InModuleName, MoveTemp(SubsystemClasses));
    }
}

void FSubsystemModuleWatcher::RemoveClassesForModule(const FName& InModuleName)
{
    TArray<TSubclassOf<UDynamicSubsystem>>* SubsystemClasses = FSubsystemCollectionBase::DynamicSystemModuleMap.Find(InModuleName);
    if (SubsystemClasses)//如果能找到，说明其之前有登记过
    {
        for (TSubclassOf<UDynamicSubsystem>& SubsystemClass : *SubsystemClasses)
        {
            FSubsystemCollectionBase::RemoveAllInstances(SubsystemClass);//销毁这个类的所有对象
        }
        FSubsystemCollectionBase::DynamicSystemModuleMap.Remove(InModuleName);//移除登记
    }
}
```

创建和销毁类的对象实例代码：

```cpp
void FSubsystemCollectionBase::AddAllInstances(UClass* SubsystemClass)
{
    for (FSubsystemCollectionBase* SubsystemCollection : SubsystemCollections)
    {
        if (SubsystemClass->IsChildOf(SubsystemCollection->BaseType))
        {   //这个函数之前解释过，用来创建对象
            SubsystemCollection->AddAndInitializeSubsystem(SubsystemClass);
        }
    }
}

void FSubsystemCollectionBase::RemoveAllInstances(UClass* SubsystemClass)
{
    ForEachObjectOfClass(SubsystemClass, //遍历属于该类型的对象们
    {
        USubsystem* Subsystem = CastChecked<USubsystem>(SubsystemObj);
        if (Subsystem->InternalOwningSubsystem)
        {   //也是大同小异的释放操作
            Subsystem->InternalOwningSubsystem->RemoveAndDeinitializeSubsystem(Subsystem);
        }
    });
}
```

这里的关键是SubsystemCollections是个静态变量，其引用了整个进程的所有定义的FSubsystemCollection的数量（2个在GEngine和GEditor里，其他多个可能动态新增在GameInstance，World，LocalPlayer里），这么写其实只是保险一点保证一个Subsystem类型的对象能够自动的多个SubsystemCollection里正确的创建多个。因为AddAllInstances也可能是个会被复用的方法。

说了一大堆，其实想说明MyPlugin模块在被加载的时候，会自动的触发OnModulesChanged事件，从而被自动的创建出内部的UMyPluginEngineSubsystem和UMyPluginEditorSubsystem。

**后期动态加载模块或插件：**

如果你不写上述的那些“静态”引用方式，想要在游戏或编辑器运行一段时间后然后动态的加载某个模块或插件。UE当然也支持，方法是：

对于模块，假如MyOtherModule是Hello项目里定义的另一个模块，你别在Build.cs添加Dependency引用，反而选择在后续的时机调用 `FModuleManager::Get().LoadModule(TEXT("MyOtherModule"));`,你就会发现MyOtherModule的StartupModule开始调用了。如果这个模块里有定义DynamicSubsystem，其会被创建出来。 对于插件，假如Hello项目不开启MyPlugin插件， **且在MyPlugin.uplugin里加上一句： `"ExplicitlyLoaded": true`** ，这一句很重要，表明后面要显式的动态加载。这样你在之后的时机里调用：

```cpp
FString path= FPaths::Combine(FPaths::ProjectPluginsDir(),TEXT("MyPlugin/MyPlugin.uplugin"));
IPluginManager::Get().AddToPluginsList(path);//添加插件路径让可以找到
IPluginManager::Get().MountExplicitlyLoadedPlugin(TEXT("MyPlugin"));//显式加载
```

这样这个插件就会被加载起来了。UMyPluginEngineSubsystem和UMyPluginEditorSubsystem也就会在这个时候被创建出来了。

**总结一下** ，MyPlugin里的DynamicSubsystem们，虽然都是靠触发OnModulesChanged事件来创建和销毁自己。但是根据你项目配置模块引用的不同，时机也可以不同。 **所以DynamicSubsystem可以根据Module的加载释放来创建销毁，就是此动态的含义！**

### 思考：动态加载MyPlugin里的UMyPluginGameInstanceSubsystem可以正常工作吗？

答案是可以。UMyPluginGameInstanceSubsystem虽然不是UDynamicSubsystem，但因为其需要创建的时机是游戏运行时。而这个时候，其实你如果去 `GetDerivedClasses(TSubclassOf&lt;UGameInstanceSubsystem&gt;)` ，你可以成功的获得到UMyPluginGameInstanceSubsystem。原因是MyPlugin这个模块dll加载的时候，其身带的反射代码里的全局静态变量会自动的在进程里注册进各种类型。这部分机制请参考我之前写的UObject文章，说来实在太话长了。所以MyPlugin已经被加载进来之后，你再点击Play，这个时候已经可以正确的使用MyPlugin里定义的非DynamicSubystem了。

### 思考：那为什么只有UEngineSubsystem和UEditorSubsystem才是UDynamicSubsystem呢？

还是得从生命周期来思考。生命周期5类中，只有UEngineSubsystem和UEditorSubsystem的生命周期是跟游戏的进程绑定在一起的。因此 **游戏进程启动的时候创建Subsystem叫做默认创建，游戏进程启动一段时间后想创建Subsystem叫做动态创建** 。而对于另外3个：UGameInstanceSubsystem，UWorldSubsystem和ULocalPlayerSubsystem，你观察一下，发现这3者都是跟一场游戏的生命周期绑定的！因此无论这3者定义上如何动态，其在编辑器启动后，都不会创建出来！而反正 **这3者会随着Play和Stop来反反复复的创建和销毁，其本身已经足够动态！** 因此就不需要专门处理了。结论还是其根据生命周期的不同而定的机制。

那前面说过了，UWorldSubsystem可能也会被WorldType::Editor的创建出来，其明显生命周期不是跟游戏一起的，编辑器如果已经完全启动完成了，那再动态加载的插件里定义的WorldSubsystem其实并不能扩展到Editor的World里。那这怎么理解呢？你可以把这当做是一种疏漏，但也可以当做是一种小故意。其含义就是：

- Engine和Editor在启动前启动后，都可以通过静态或动态加载插件来扩展。
- Game的部分，如果想扩展的话，就尽量在Game启动之前。

理论上你当然可以做一些骚操作，在游戏运行一半的时候，动态加载某个插件，然后再下个World里应用上你新增的WorldSubsystem，但实际情况是没这必要这么麻烦，因为已经有ShouldCreateSubsystem可以让你控制了。

**四，理解Subsystem对象的个体生命周期**

这一部分比较简单，就是你要理解好一个Subsystem对象：ShouldCreateSubsystem，Initialize和Deinitialize的调用时机才知道应该怎么重载。

![](https://picx.zhimg.com/v2-933729259852ecba032db5b19a6ebf4f_1440w.jpg)

要明白USubsystem本身是一个UObject，所以必然有CDO。自己用的时候要注意这一点。

**五，总结一下其生命周期的不同时机**

絮叨了一堆，希望读者们能清晰明白到生命周期其关键点就在于什么时候触发SystemCollection的Initiaze和DeInitialize，根据Outer对象自身运行机制生命周期的不同，由此搭配出不同的使用方式。在理解了这些不同Subsystem对象的不同之后，也许你也可以由此组织实现符合自己需求的加载创建策略。

![](https://pic2.zhimg.com/v2-69546312c4c3c05093f94f12b60904cf_1440w.jpg)

## 如何用好Subsystems？

如果说到学习使用Subsystems的难点是什么，那肯定是如何用好它了！Subsystems本身是机制和使用方式并不复杂，但反而容易令人疑惑的是： **该把自己项目的哪些业务逻辑封装成Subsystem？**

一谈到编程实践，笔者本身并无金科玉律可传授，但可分享一些理解的点和使用的经验：

**1\. Subsystem是GamePlay级别的Component。** 要好好理解这句话的关键是，你要明白：

- Component组件模式在程序架构中的作用套路，组合胜过继承而带来的灵活性。
- 清晰区分通用功能和业务逻辑。对于一个游戏来说，通用功能可能是人物按格子行走的 **能力** ，而业务逻辑是这个人物自身的战斗结算系统。通常来说，我倾向于把功能理解为可在不同游戏之间复用的功能，尽量与某款特定游戏无关，而与某类游戏相关，比如CharacterMovement。那自然的，剩下的与某款游戏强相关的部分，就是该游戏的业务逻辑部分了。对于 **功能能力** ，你应该用ActorComponent来实现。对于 **业务逻辑** ，你应该用Subsystem来实现。更清晰的对比是：
- ActorComponent：依托Actor存在，封装基础的功能，着眼于某一功能的实现。
	- Subsystem：依托GamePlay对象存在，封装游戏的业务逻辑，专注于某部分游戏逻辑系统的调度安排。
	- 二者是配合补充的关系。Component在底层专注于功能，Subsystem在上层统筹调度。虽然二者也可以有一些交集，可以替代着实现一些功能，但笔者并不建议如此。

**2\. USubsystem只是个普通的UObject。** 所以，别害怕，USubsystem并没有什么特殊的，你之前可以在UObject里写的东西，可以实现的功能，依然可以在USubsystem里实现。所以你可以在里面写UPROPERTY和UFUNCTION，也可以定义事件，也可以被蓝图继承引用等等。只是心里永远有个意识，Subsystem的生命周期处于你的掌控之中。

**3\. Subsystem是有状态的。** 很多人可能还发现其跟蓝图函数库有点相像，但其最大的区别是蓝图函数库本身全都是静态函数，是无状态的。而Subsystem对象里面可以放属性，会真正的被构建出对象来，因此是有状态的。理论上你也是可以对此序列化的。

**4\. 尽量不要再写Manager类了。** 在以前你或许会自己定义一些Manager类，比如MapManager之类的，但现在都可以换成UMapWorldSubsystem了。理由是之前我们定义Manager类，无非是想要有一个全局的统一管理类来统筹做一些调度管理，现在这部分工作可以由Subsystem来做了。自己定义的Manager类的理由和空间越来越小了。

**5\. 引擎源码里已经定义了一些Subsystem，方便你抄袭学习。** UE引进了Subsystem并不是只对外提供给开发者使用的，其内部也自己先吃了Dogfood，把一些功能改造成Subsystem了。简单列一下列表。具体的功能不解释了，有兴趣自己去模板。

- UEditorSubsystem
- UnrealEd.AssetEditorSubsystem
	- UnrealEd.BrushEditingSubsystem
	- UnrealEd.ImportSubsystem
	- UnrealEd.LayersSubsystem
	- UnrealEd.PanelExtensionSubsystem
	- MovieRenderPipelineEditor.MoviePipelineQueueSubsystem
	- Blutility.EditorUtilitySubsystem
	- GeometryMode.UBrushEditingSubsystem
- GeometryMode.UBrushEditingSubsystemImpl
	- VirtualProductionUtilities.UVPScoutingSubsystem
	- DataValidation.UEditorValidatorSubsystem
- UWorldSubsystem
- Landscape.LandscapeSubsystem
	- Engine.AutoDestroySubsystem
	- Engine.ObjectTraceWorldSubsystem

## 使用Subsystems的各种姿势

继续分享一些能想到的编写Subsystems的套路：

**1\. 可定义一个Subsystem抽象基类，然后派生多个子类。**

用这种方式，依然可以在Subsystem的继承体系里往上抽象一些基本逻辑。官方直播里举的一个例子我觉得特别的形象，假如你要实现版本控制系统，且同时支持多个协议。这时你就可以定一个 `UCLASS(abstract) USourceControlSubsystem` ，然后往下派生出UGitSubsystem，USVNSubsystem，UPerforceSubsystem等等。原因是这些版本管理系统是拥有一些共同机制的。但请注意在基类上务必加上abstract的标记，以防止其也被实例化出来，那就很容易引起混淆了。 然后引擎还支持Subsystem的遍历：

```cpp
const TArray<USourceControlSubsystem*>& systems= GEngine->GetEngineSubsystemArray<USourceControlSubsystem>();
```

这样就可以获得其多个git，svn等Subsystem实例了。

**2\. Subsystem也支持蓝图继承！**

目前虽然不支持直接在蓝图里定义Subsystem，究其原因是生命周期5类定义的时候UCLASS里都没加上Blueprintable标记。但这一步其实很容易绕过，只要你自己定义的Subsystem上的UCLASS里加上标记：

```cpp
UCLASS(abstract, Blueprintable, BlueprintType)
class HELLO_API UMyGameInstanceSubsystemBase : public UGameInstanceSubsystem
{
}
```

这样就可以在蓝图里继承然后使用了。依然加上abstract的用意是防止UMyGameInstanceSubsystemBase被实例化出来，造成混淆。Blueprintable和BlueprintType是支持蓝图继承和定义变量。

注意：抱歉，之前没有写清楚。有些人反馈遇见了蓝图Subsystem无法创建的问题。这个问题的原因是，如果你的蓝图类只是创建出来，但是在还没有被加载的时候，这个蓝图类的类型其实还没有注册到UE的类型系统里。因此就导致了SubsystemCollection在搜集和创建的时候，无法识别到该蓝图类，因此就无法创建出来。特别要提醒注意的是，这个问题很隐晦，很多人可能会发现有时可以创建有时失败，很有可能只是因为有时你在UE编辑器里双击打开了这个Subsystem蓝图类，而有时没有。在编辑器里双击打开蓝图类其实就会触发BP的加载，因此会发现PIE的时候就能正确创建了，而有时你没打开，就会创建失败。

理解了原因之后，解决的方式其实就很简单了，只要我们手动的触发一个Load，就可以了。

```cpp
void FHelloModule::StartupModule()
{
    UObject* bpAsset = LoadObject<UObject>(NULL, TEXT("/Game/BP_MyGameInstanceSubsystem.BP_MyGameInstanceSubsystem_C"));
}
```

因此假如我们在模块的StartupModule里自己手动加载一下这个蓝图，其实就可以创建成功了。

**3\. 同一个BaseType下的多个Subsystem可以定义依赖顺序**

虽然不太推荐在Subsystem之间构成强顺序依赖，但万一你真的需要的话，比如任务系统依赖于计分系统的计算结果来决定是否触发某个任务。这个时候，你可以这么写：

```cpp
void UMyTaskSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    //初始化依赖项
    Collection.InitializeDependency(UMyScoreSubsystem::StaticClass());
    //获取
    UMyScoreSubsystem* ss=((FSubsystemCollection<UGameInstanceSubsystem>&)Collection).GetSubsystem<UMyScoreSubsystem>(UMyScoreSubsystem::StaticClass());
}
```

InitializeDependency内部会调用一下AddAndInitializeSubsystem来确保UMyScoreSubsystem已经被创建出来。这样你在后续就可以访问了。现在的访问方式依然有点别扭，希望后续可以改善。

**4\. Subsystem之间也可以通信**

只要你获取到目标Subsystem的对象引用，你就可以使用它了。获取的方式有多种，一种是通过GEngine或GetGameInstance()等外部对象开始，然后GetSubsystem()来特定的获取。另一种是通过Subsystem在Initialize的时候，保存下其他Subsystem对象的指针来后续访问。

**5\. Subsystem也可以支持Tick**

参考源码里的UAutoDestroySubsystem，咱们自己写一个支持Tick的Subsystem可以继承自FTickableGameObject：

```cpp
UCLASS()
class HELLO_API UMyTickSubsystem : public UGameInstanceSubsystem,public FTickableGameObject
{
    GENERATED_BODY()
public:
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return !IsTemplate(); }//不是CDO才Tick
    virtual TStatId GetStatId() const override{RETURN_QUICK_DECLARE_CYCLE_STAT(UMyScoreSubsystem, STATGROUP_Tickables);}
};
```

其中IsTickable的实现要注意一下避免CDO，否则会造成Tick两次。

**6\. SubSystem里定义一些委托回调是一个常用的套路**

还是以Score系统为例子，Subsystem往往自身都带有一些业务数据。而当这些数据改变的时候，往往也需要通知一些对象如UI来更新状态。这个时候可以定义一些委托回调：

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScoreChanged, float, NewScore);

UPROPERTY(BlueprintAssignable)
FScoreChanged OnScoreChanged;

UFUNCTION(BlueprintCallable)
void AddScore(float delta)
{
    Score += delta;
    OnScoreChanged.Broadcast(Score);
}
```

这样OnScoreChanged就可以在蓝图和其他地方BindEvent了。因为Subsystem本身具有全局访问的特性，因此往往很容易作为数据的集中式更新和事件的统一分发处。

## 再来一些零散的思考吧

还有一些小的思考，也想分享一下。作为一个程序架构设计的思考领悟交流。

### 思考：Subsystem支持网络复制吗？

不支持。我们知道UE里的网络复制是基于Actor的ActorChannel的，而USubsystem是普通的UObject对象，因此并不支持。在这一点上，你可能会有冲动去定义一个AManagerActor类来作为通信通道，这也许是一个好主意，但也得仔细的评估。因为常常很多时候，GameState和PlayerState等一些内建的GamePlay通信已经能够满足你的需求，我比较建议尽量把职责划分清楚。实在不行的话，那就用一个Actor来作为通信通道吧，反正再坏也坏不到哪里去，是吧。

### 思考：Subsystem为什么单挑这5个类？

多问几个为什么，不免会此疑问。我自己在思考这个问题的时候，会从引擎的关键流程有几个来思考。Engine支持了通用的引擎时机，Editor专门支持了编辑器。接下来的游戏启动和关闭，有GameInstance来负责了。然后游戏中的切换关卡有World相关联了。本地多玩家由LocalPlayer来支持。你会发现，这些对象都是关键的数据节点。

有人可能会问那怎么不弄UGameModeSubsystem？怎么不弄UPlayerControllerSubsystem？我尝试回答的答案是要控制拆分的粒度。理论上你甚至可以再定义UPawnSubsystem，把角色身上的业务逻辑也垂直拆分一下，但这未免就有点太多太细了，在很多时候往往都用不上。业务逻辑粒度太小了就更难拆分出来了。因此设计的一个原则是合适的考虑实际需求，不宜妄自追求灵活性。另一个方面，GameMode和PlayerController这些对象的生命周期往往是和World一起的，得到了World就可以顺藤摸瓜得到其他，因此在功能实现上倒也不损失多少。

当然，我同时的观点是引擎是一个持续迭代的产物，未来也许也会根据需要继续添加其他。未来需要构建更大的大世界后，架构必须演化以适应，自然会做更多的自调整。

### 思考：可否自己也参照着新建个USubsystem基类？

源码在手，为所欲为。在理解了这套机制的基础上，自己读读源码，其实不难参照着自己写一套。只是我自己确实没有想到实际的游戏需求。关键的倒不是如何继续添加一个，而是在吸收了这些架构知识营养后，读者们可以在自己的游戏结构里灵活应用上对象的反射和事件注册等知识。

### 思考：好奇蓝图里是怎么访问到Subsystem全局变量的？

关于这一点，引擎其实是做了专门的实现的。具体的源码原理可以参照，UK2Node\_GetSubsystem定义了专门的蓝图节点，其内部实现会转发到USubsystemBlueprintLibrary的一些静态函数接口去。因此关于怎么理解Subsystem的上下文ContextObject这个问题，读者朋友们可以好好的读下USubsystemBlueprintLibrary的源码。其内容比较简单直接，我就不赘述贴出来了。

## 总结

不好意思，本文篇幅实在太长。原因是我想毕其功于一文，希望一次就能讲透。Subsystems其实可以算是GamePlay框架上发布的一个美味DLC，懂得使用它的人会让自己的程序架构愈发的清晰，而不懂的的人依然习惯把代码搅和在一团，摊手~

希望本文对大家有帮助，没脸皮求赞了，就酱，后会有期！

## 引用

1. [Programming Subsystems官方文档](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/en-US/Programming/Subsystems/index.html)
2. [\[英文直播\]Programming Subsystems(真实字幕组)](https://link.zhihu.com/?target=https%3A//www.bilibili.com/video/BV1t741187FZ)

*UE4.25.1*

——————————————————————————————————————

知乎专栏： [InsideUE4](https://zhuanlan.zhihu.com/insideue4)

UE深入学习QQ群： **456247757** (非新手入门群，请先学习完官方文档和视频教程)

**个人原创，未经授权，谢绝转载！**

编辑于 2022-05-08 17:10[虚幻引擎](https://www.zhihu.com/topic/19824201)[游戏引擎](https://www.zhihu.com/topic/19556258)[游戏开发](https://www.zhihu.com/topic/19553361)