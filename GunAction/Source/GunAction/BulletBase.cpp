/*
   - BulletBase -
   作成: 怜旺.

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
    //
    //GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, msg); //表示.

    //敵に当たったなら.(AActor型をAEnemyManager型にキャスト)
    if (AEnemyManager* enm = Cast<AEnemyManager>(OtherActor)) {
        enm->OnBulletHit(); //敵の死亡処理.
        Destroy();          //弾消滅.
    }
}

/// <summary>
/// 常に実行.
/// </summary>
void ABulletBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    const FVector befPos = GetActorLocation();              //移動前の座標.
    {   
        vec.Z -= gravity;                                   //だんだん下へ.
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

    counter += 1; //カウンター.
    if (counter >= deleteTime) {
        Destroy();
    }
}

/// <summary>
/// 発射処理 | 進行方向を指定.
/// </summary>
/// <param name="forward">進行方向</param>
void ABulletBase::ShotForward(FVector forward) {
	this->vec = forward; //進行方向を記録.
}
/// <summary>
/// 発射処理 | 目標地点を指定.
/// </summary>
/// <param name="position">目標座標</param>
void ABulletBase::ShotPos(FVector position) {

    //距離差を求める.
    FVector dist = position - GetActorLocation();
    //正規化(単位ベクターにする)
    dist.Normalize();

    this->vec = dist; //進行方向を記録.
}