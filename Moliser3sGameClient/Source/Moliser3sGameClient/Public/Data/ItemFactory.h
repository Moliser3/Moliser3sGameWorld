#pragma once

#include "CoreMinimal.h"
#include "ItemFactory.generated.h"

class UItemBase;
class UEquipItem;
class UConsumableItem;
class UDataTable;

UCLASS()
class MOLISER3SGAMECLIENT_API UItemFactory : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "物品", meta = (WorldContext = "WorldContextObject"))
	static UEquipItem* CreateEquipment(const UObject* WorldContextObject, UDataTable* DataTable, FName RowID);

	UFUNCTION(BlueprintCallable, Category = "物品", meta = (WorldContext = "WorldContextObject"))
	static UConsumableItem* CreateConsumable(const UObject* WorldContextObject, UDataTable* DataTable, FName RowID);

	UFUNCTION(BlueprintCallable, Category = "物品", meta = (WorldContext = "WorldContextObject"))
	static UItemBase* CreateMaterial(const UObject* WorldContextObject, UDataTable* DataTable, FName RowID);

	UFUNCTION(BlueprintCallable, Category = "物品", meta = (WorldContext = "WorldContextObject"))
	static UItemBase* CreateQuestItem(const UObject* WorldContextObject, UDataTable* DataTable, FName RowID);
};
