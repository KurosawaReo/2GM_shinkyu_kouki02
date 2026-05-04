/*
   - BulletBase -
   担当: 怜旺

   弾の基底クラス.
   そのうち継承して使うことを想定.
*/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "NiagaraSystem.h" //エフェクト用.

#include "BulletBase.generated.h"

/*
	[UPROPERTY()マクロ]

	Unityの[SerializeField]のようなもの.

	Blueprint側から値をいじれるようになり
	EventGraphでSet/Getをできるようになる.

	UPROPERTY(1, 2, 3)
	1: Blueprint内で編集する際の表示・編集の状態.
	2: ReadOnly or ReadWrite.
	3: 何のカテゴリに入れるか(自由名)
*/

//前方宣言.
class ACharacterBase;

/// <summary>
/// 弾の使用者.
/// </summary>
UENUM(BlueprintType)
enum class EBulletUser : uint8
{
	None   UMETA(DisplayName = "None"),  //誰の弾でもない.
	Enemy  UMETA(DisplayName = "Enemy"),
	Player UMETA(DisplayName = "Player")
};

/// <summary>
/// 弾の基底クラス.
/// </summary>
UCLASS()
class GUNACTION_API ABulletBase : public AActor
{
	GENERATED_BODY()

//▼ ===== コンポーネント ===== ▼.
private:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* cmpSphere; //球体.

//▼ ===== 変数 ===== ▼.
private:
	FVector vec;      //進行方向.
	int     counter;  //経過時間.

	UPROPERTY()
	EBulletUser user; //誰が撃った弾か.

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty")
	float   speed      = 1;  //速度.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty")
	float   deleteTime = 50; //消滅までの時間.

	UPROPERTY(EditAnywhere, Category = "MyProperty|Effect")
	UNiagaraSystem* TrailEffectAsset; //弾道エフェクト.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Effect")
	int TrailSpawnStep = 2; //何フレームごとに出すか.

//▼ ===== 関数 ===== ▼.
public:	
	//コンストラクタ.
	ABulletBase();

protected:
	//召喚時に呼ばれる.
	virtual void BeginPlay() override;

	//当たった時に呼ばれる.
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult
	);

public:	
	//set.
	virtual void SetUser(EBulletUser _user);
	//常に実行.
	virtual void Tick(float DeltaTime) override;
	//弾道召喚.
	void SpawnTrail();
};