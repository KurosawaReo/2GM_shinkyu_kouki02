/*
   - BulletBase -
   作成: 怜旺.

   弾の元となる基底クラス.
*/
#include "BulletBase.h"
#include "Enemy.h"

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

    //敵に当たったなら.(AActor型をAEnemy型にキャスト)
    if (AEnemy* enm = Cast<AEnemy>(OtherActor)) {
        //敵の死亡処理.
        enm->OnBulletHit();
    }
}

/// <summary>
/// 常に実行.
/// </summary>
void ABulletBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    //FString msg = FString::Printf(TEXT("vec: %f %f %f"), vec.X, vec.Y, vec.Z); //変数組み込み.
    //GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, msg); //表示.

	//前方向に移動.
	SetActorLocation(GetActorLocation() + vec * speed);
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