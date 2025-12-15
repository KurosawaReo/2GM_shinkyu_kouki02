/*
   - BulletBase -
   担当: 怜旺

   弾の元となる基底クラス.
*/
#include "BulletBase.h"
#include "EnemyManager.h"

/// <summary>
/// コンストラクタ.
/// </summary>
ABulletBase::ABulletBase()
{
	PrimaryActorTick.bCanEverTick = true;

    //初期化.
    speed = 0;
    vec   = FVector();
    
    //コンポーネント作成.
    cmpSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    //ルートにする.
    RootComponent = cmpSphere;

    //BeginOverlap: 当たった瞬間に呼ばれる関数をセット.
    cmpSphere->OnComponentBeginOverlap.AddDynamic(this, &ABulletBase::OnOverlapBegin);
}

/// <summary>
/// 召喚時に呼ばれる.
/// </summary>
void ABulletBase::BeginPlay()
{
	Super::BeginPlay();

    //前方向を取得.
    vec = GetActorForwardVector();
}

/// <summary>
/// 当たった時に呼ばれる.
/// </summary>
void ABulletBase::OnOverlapBegin(
    UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp,      int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult
){   
    //FString name = OtherActor->GetName();                   //名前取得.
    //FString msg  = FString::Printf(TEXT("Hit: %s"), *name); //変数組み込み.
    //GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, msg); //表示.

    //撃った人がプレイヤー.
    if (Cast<APlayerManager>(user)) {
        //敵に当たった.
        if (auto enm = Cast<AEnemyManager>(OtherActor)) {
            enm->OnBulletHit(); //被弾処理.
            Destroy();          //弾消滅.
        }
    }
    //撃った人が敵.
    if (Cast<AEnemyManager>(user)) {
        //プレイヤーに当たった.
        if (auto ply = Cast<APlayerManager>(OtherActor)) {
            ply->OnBulletHit(); //被弾処理.
            Destroy();          //弾消滅.
        }
    }
}

/// <summary>
/// set.
/// </summary>
/// <param name="user">撃った人のクラス</param>
void ABulletBase::SetUser(AActor* _user) {
    user = _user; //銃を撃った人を登録.
}

/// <summary>
/// 常に実行.
/// </summary>
void ABulletBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    const FVector befPos = GetActorLocation();              //移動前の座標.
    {   
        vec.Z -= gravity;                                   //重力.
        SetActorLocation(GetActorLocation() + vec * speed); //前方向に移動.
    }
    const FVector nowPos = GetActorLocation();              //移動後の座標.
     
    //弾の軌道.
    DrawDebugLine(
        GetWorld(),
        befPos,
        nowPos,
        FColor(255, 0, 0), // 線の色
        false,             // 永続かどうか
        1.0f,              // 表示時間
        0,
        1.0f               // 太さ
    );

    //カウンター.
    counter += 1;
    //一定時間経過で消滅.
    if (counter >= deleteTime) {
        Destroy();
    }
}