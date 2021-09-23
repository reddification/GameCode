#pragma once

DECLARE_DELEGATE_OneParam(FAmmoChangedEvent, int32 ClipAmmo);
DECLARE_DELEGATE_TwoParams(FWeaponAmmoChangedEvent, int32 ClipAmmo, int32 RemainingAmmo);
