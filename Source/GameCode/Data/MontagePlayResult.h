#pragma once

struct FMontagePlayResult
{
	bool bStarted = false;
	class UAnimMontage* AnimMontage = nullptr;
	class UAnimInstance* AnimInstance = nullptr;
	bool bDisableCameraRotation = false;
};
