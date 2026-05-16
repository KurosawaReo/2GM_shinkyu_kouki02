/*
   - BulletBase -
   担当: 怜旺

   弾の元となる基底クラス.
*/
#include "BulletBase.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

//他class.
#include "PlayerCharacter.h"
#include "EnemyCharacter.h"
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
    team    = ETeam::None; //チームなし.
    
    //コンポーネント作成.
    cmpSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    //ルートにする.
    RootComponent = cmpSphere;

    //BeginOverlap: 当たった瞬間に呼ばれる関数をセット.
    cmpSphere->OnComponentBeginOverlap.AddDynamic(this, &ABulletBase::OnOverlapBegin);
}

/// <summary>
/// 召喚した瞬間に実行.
/// </summary>
void ABulletBase::BeginPlay()
{
	Super::BeginPlay();

    //前方向を取得.
    vec = GetActorForwardVector();
}

/// <summary>
/// 何かに当たった時に実行.
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

    //チーム別.
    switch (team) 
    {
        //撃った人がプレイヤー.
        case ETeam::Player:
            //敵にヒット.
            if (auto enm = Cast<AEnemyCharacter>(OtherActor)) {
                enm->Damage(damage); //ダメージ処理.
                Destroy();           //弾消滅.
            }
            break;

        //撃った人が敵.
        case ETeam::Enemy:
            //プレイヤーにヒット.
            if (auto ply = Cast<APlayerCharacter>(OtherActor)) {
                ply->Damage(damage); //被弾処理.
                Destroy();           //弾消滅.
            }
            break;
    }
}

/// <summary>
/// 常に実行.
/// </summary>
void ABulletBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //前方向に移動.
    SetActorLocation(GetActorLocation() + vec * speed, true);
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
/// チームを登録.
/// </summary>
void ABulletBase::SetTeam(ETeam _team) {
    team = _team;
}

/// <summary>
/// 弾道の生成.
/// </summary>
void ABulletBase::SpawnTrail() {

    UNiagaraSystem* effect = nullptr;

    //チーム別.
    switch (team)
    {
        //撃った人がプレイヤー.
        case ETeam::Player:
            effect = effectTrailPlayer;
            break;

        //撃った人が敵.
        case ETeam::Enemy:
            effect = effectTrailEnemy;
            break;
    }

    //弾道.
    if (effect) {

        //生成位置.
        const FVector spawnPos = GetActorLocation();
        //生成.
        UNiagaraComponent* comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), effect, spawnPos);

        if (comp)
        {
            comp->SetAutoDestroy(true);

            //一定時間で消滅.
            FTimerHandle handle;
            GetWorld()->GetTimerManager().SetTimer(
                handle,
                [comp]()
                {
                    if (comp)
                    {
                        comp->Deactivate(); //再生停止.
                    }
                },
                0.2f, //消えるまでの時間.
                false
            );
        }
    }
}