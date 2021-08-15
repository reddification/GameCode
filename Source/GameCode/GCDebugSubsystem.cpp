// Fill out your copyright notice in the Description page of Project Settings.


#include "GCDebugSubsystem.h"

bool UGCDebugSubsystem::IsDebugCategoryEnabled(const FName& CategoryName) const
{
	const bool* bEnabled = CategoriesStates.Find(CategoryName);
	return bEnabled && *bEnabled;
}

void UGCDebugSubsystem::SetDebugCategoryEnabled(const FName& CategoryName, bool bEnabled)
{
	bool& bCategoryEnabled = CategoriesStates.FindOrAdd(CategoryName);
	bCategoryEnabled = bEnabled;
}

