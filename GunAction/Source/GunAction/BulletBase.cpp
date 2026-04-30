/*
   - BulletBase -
   担当: 怜旺

   弾の元となる基底クラス.
*/
#include "BulletBase.h"

//他class.
#include "PlayerManager.h"
#include "EnemyManager.h"
#include "CharacterBase.h"

/// <summary>
/// コンストラクタ.
/// </summary>
ABulletBase::ABulletBase()
{
	PrimaryActorTick.bCanEverTick = true;

    //初期化.
    speed   = 0;
    vec     = FVector::ZeroVector;
    counter = 0.0f;
    user    = EBulletUser::None; //使用者なし.
    
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

    if (!IsValid(OtherActor)) return;
    if (OtherActor == this) return;

    //ユーザー別.
    switch (user) 
    {
        //撃った人がプレイヤー.
        case EBulletUser::Player:
            //敵にヒット.
            if (auto enm = Cast<AEnemyManager>(OtherActor)) {
                enm->OnBulletHit(); //被弾処理.
                Destroy();          //弾消滅.
            }
            break;

        //撃った人が敵.
        case EBulletUser::Enemy:
            //プレイヤーにヒット.
            if (auto ply = Cast<APlayerManager>(OtherActor)) {
                ply->OnBulletHit(); //被弾処理.
                Destroy();          //弾消滅.
            }
            break;
    }
}

/// <summary>
/// 銃を撃った人を登録.
/// </summary>
void ABulletBase::SetUser(EBulletUser _user) {
    user = _user;
}

/// <summary>
/// 常に実行.
/// </summary>
void ABulletBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    const FVector befPos = GetActorLocation();              //移動前の座標.
    {   
        SetActorLocation(GetActorLocation() + vec * speed); //前方向に移動.
    }
    const FVector nowPos = GetActorLocation();              //移動後の座標.
     
    FColor color;

    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("ugoita"));

    //ユーザー別.
    switch (user)
    {
        //撃った人がプレイヤー.
        case EBulletUser::Player:
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("player"));
            color = FColor(0, 255, 255);
            break;

        //撃った人が敵.
        case EBulletUser::Enemy:
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("enemy"));
            color = FColor(255, 0, 0);
            break;
    }

    //弾の軌道.
    DrawDebugLine(
        GetWorld(),
        befPos,
        nowPos,
        color,  //線の色.
        false,  //永続かどうか.
        1.0f,   //表示時間.
        0,
        1.0f    //太さ.
    );

    //カウンター.
    counter += 1;
    //一定時間経過で消滅.
    if (counter >= deleteTime) {
        Destroy();
    }
//  FString text = FString::Printf(_T("counter:%f"), counter);
//  GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, text);
}