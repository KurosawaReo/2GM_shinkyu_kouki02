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
	FVector vec;     //進行方向.
	float   counter; //経過時間.

	UPROPERTY()
	TObjectPtr<AActor> user; //誰が撃った弾か.

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MyProperty)
	float   speed      = 1;  //速度.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MyProperty)
	float   deleteTime = 50; //消滅までの時間.

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
	virtual void SetUser(TObjectPtr<AActor> user);
	//常に実行.
	virtual void Tick(float DeltaTime) override;
};