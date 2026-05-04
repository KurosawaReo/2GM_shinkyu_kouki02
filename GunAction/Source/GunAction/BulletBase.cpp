/*
   - BulletBase -
   担当: 怜旺

   弾の元となる基底クラス.
*/
#include "BulletBase.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

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

    //前方向に移動.
    SetActorLocation(GetActorLocation() + vec * speed);
    //弾道の生成.
    SpawnTrail();

    //カウンター.
    counter += 1;
    //一定時間経過で消滅.
    if (counter >= deleteTime) {
        Destroy();
    }
}

/// <summary>
/// 弾道の生成.
/// </summary>
void ABulletBase::SpawnTrail() {

    //ユーザー別.
    switch (user)
    {
        //撃った人がプレイヤー.
        case EBulletUser::Player:
        break;

        //撃った人が敵.
        case EBulletUser::Enemy:
        break;
    }

    //弾道.
    if (TrailEffectAsset) {

        //生成させるか判定.
        bool isSpawn = false;
        //0割対策.
        if (TrailSpawnStep != 0) {
            isSpawn = (counter % TrailSpawnStep == 0);
        }
        else {
            isSpawn = false;
        }

        //一定間隔で生成.
        if (isSpawn)
        {
            //生成位置.
            const FVector spawnPos = GetActorLocation();
            //生成.
            UNiagaraComponent* comp =
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                    GetWorld(),
                    TrailEffectAsset,
                    spawnPos
                );

            if (comp)
            {
                comp->SetAutoDestroy(true);

                // タイマーで停止
                FTimerHandle handle;
                GetWorld()->GetTimerManager().SetTimer(
                    handle,
                    [comp]()
                    {
                        if (comp)
                        {
                            comp->Deactivate(); // 再生停止
                        }
                    },
                    0.2f, // ←寿命
                    false
                );
            }
        }
    }
}