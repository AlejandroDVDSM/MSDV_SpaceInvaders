// Fill out your copyright notice in the Description page of Project Settings.


#include "SIGameInstance.h"

int64 USIGameInstance::GetRecord()
{
	return playerRecord;
}

void USIGameInstance::UpdateRecord(int64 newRecord)
{
	playerRecord = newRecord;
}
