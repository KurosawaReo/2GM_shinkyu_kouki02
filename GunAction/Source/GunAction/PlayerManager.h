/*
   - PlayerManager -
   共通のCharacterクラスから派生したプレイヤークラス.

   [クラス構成]
   CharacterBase
   └PlayerManager ←今ここ
   └EnemyManager
*/
#pragma once
#include "CoreMinimal.h"

//他class.
#include "CharacterBase.h" //親クラス.
#include "Steam_Revolver.h"

#include "PlayerManager.generated.h"
class UCrosshairWidget;
class USpringArmComponent;
class UCameraComponent;

/// <summary>
/// プレイヤークラス.
/// </summary>
UCLASS()
class GUNACTION_API APlayerManager : public ACharacterBase
{
	GENERATED_BODY()

//▼ ===== 変数 ===== ▼.
public:

	//カメラコンポーネント
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|Camera")
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|Camera")
	class UCameraComponent* FollowCamera;

	//▼ ===== IKアニメーション用の変数 ===== ▼
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|IK")
	float IKAlpha = 1.0f; // IKの強度（0.0～1.0）

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|IK")
	FVector RightHandIKLocation = FVector::ZeroVector; // 右手のIK目標位置（ワールド座標）

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|IK")
	FVector RightElbowIKLocation = FVector::ZeroVector; // 肘のIK目標位置（オプション）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|IK")
	FName RightHandSocketName = FName("upperarm_r");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|IK")
	float IKDistance = 100.0f;//まずるからの距離.

	//プレイヤーの腕処理.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|IK")
	bool bEnbLeArmIK = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|IK")
	FVector RightHandIKTarget = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|IK")
	float RightHandIKAlpha = 0.0f;

//▼ ===== 関数 ===== ▼.
protected:

#pragma region "基本処理"
	APlayerManager();						//コンストラクタ.
	void BeginPlay() override;				//召喚した瞬間.
	void Tick(float DeltaTime) override;	//常に実行.
#pragma endregion

public:
#pragma region "入力処理"
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//入力バインド.
	void Input(UInputComponent* PlayerInputComponent);
#pragma endregion

#pragma region "移動"
	void OnMoveForward(float Value);
	void OnMoveRight(float Value);
	void OnTurnRate(float Rate);
	void OnLookUpRate(float Rate);
#pragma endregion

#pragma region "射撃"
	void OnFire();					//射撃処理.
	void ShotExe() override;		//射撃実行.
#pragma endregion

#pragma region "ダメージ・死亡"
	void OnBulletHit() override;	//弾が当たったら実行される.
	void Death()       override;	//死亡処理.
#pragma endregion

#pragma region "UI"
	void InitUI();
#pragma endregion
};