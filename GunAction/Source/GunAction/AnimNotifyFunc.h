/*
   - AnimNotifyFunc -
  
   Notifyという機能。
   アニメーション中に関数を実行することができる。
*/
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotifyFunc.generated.h"

/// <summary>
/// 射撃アニメーション中に呼び出される.
/// </summary>
UCLASS()
class GUNACTION_API UAnimNotifyFunc : public UAnimNotify
{
	GENERATED_BODY()

public:
	//関数名と引数はこれで固定.
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};