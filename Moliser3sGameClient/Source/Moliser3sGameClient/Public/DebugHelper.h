#pragma once

#include "CoreMinimal.h"

#ifndef DEBUG_ENABLED
#define DEBUG_ENABLED 1
#endif

#if DEBUG_ENABLED
#define DEBUG_LOG(Format, ...) \
	UE_LOG(LogTemp, Warning, TEXT(Format), ##__VA_ARGS__); \
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, \
		FString::Printf(TEXT(Format), ##__VA_ARGS__));
#else
#define DEBUG_LOG(Format, ...)
#endif

#define DEBUG_LOG_CUSTOM(Color, Duration, Format, ...) \
	UE_LOG(LogTemp, Warning, TEXT(Format), ##__VA_ARGS__); \
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, Duration, Color, \
		FString::Printf(TEXT(Format), ##__VA_ARGS__));

#define DEBUG_LOG_KEY(Key, Duration, Color, Format, ...) \
	UE_LOG(LogTemp, Warning, TEXT(Format), ##__VA_ARGS__); \
	if (GEngine) GEngine->AddOnScreenDebugMessage(Key, Duration, Color, \
		FString::Printf(TEXT(Format), ##__VA_ARGS__));

#define DEBUG_SCREEN(Color, Duration, Format, ...) \
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, Duration, Color, \
		FString::Printf(TEXT(Format), ##__VA_ARGS__));
