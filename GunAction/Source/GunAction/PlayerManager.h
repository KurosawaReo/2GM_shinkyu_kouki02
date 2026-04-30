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

#pragma region "カメラ"
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|Camera")
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Player|Camera")
	class UCameraComponent* FollowCamera;
#pragma endregion

#pragma region "UI"
	//クロスヘア.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Base|UI")
	TSubclassOf<UCrosshairWidget> CrosshairWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MyProperty|Base|UI")
	UCrosshairWidget* CrosshairWidget;
#pragma endregion
#pragma region "ジャンプ"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyProperty|Player|Animation")
	UAnimMontage* JumpMontage;      // ジャンプモンタージュ.
#pragma endregion

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

#pragma region"ジャンプ".
	void Jump() override;//ジャンプ開始.

	void StopJumping() override;//ジャンプ終了.

	void UpdateJumpAnimation();//ジャンプアニメーション更新.

	bool bWasInAir = false; //前フレームの空中フラグ.
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