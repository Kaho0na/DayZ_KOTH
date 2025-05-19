/**
 * ExpansionDeadDropItemData.c
 *
 * This class defines the structure and data handling
 * for individual items stored in a DeadDrop record.
 */

class ExpansionDeadDropItemData
{
	string type;
	float health;
	float quantity;
	string liquidType;
	int ammo;
	int skinIndex;
	bool wasInHands;
	string slotName;

	ref array<ref ExpansionDeadDropItemData> attachments;
	ref array<ref ExpansionDeadDropItemData> cargo;

	// Constructor
	void ExpansionDeadDropItemData()
	{
		attachments = new array<ref ExpansionDeadDropItemData>();
		cargo = new array<ref ExpansionDeadDropItemData>();
	}

	// Deep copy
	void CopyFrom(ExpansionDeadDropItemData item)
	{
		type = item.type;
		health = item.health;
		quantity = item.quantity;
		liquidType = item.liquidType;
		ammo = item.ammo;
		skinIndex = item.skinIndex;
		wasInHands = item.wasInHands;
		slotName = item.slotName;

		attachments = new array<ref ExpansionDeadDropItemData>();
		foreach (ExpansionDeadDropItemData att : item.attachments)
		{
			ExpansionDeadDropItemData newAtt = new ExpansionDeadDropItemData();
			newAtt.CopyFrom(att);
			attachments.Insert(newAtt);
		}

		cargo = new array<ref ExpansionDeadDropItemData>();
		foreach (ExpansionDeadDropItemData carg : item.cargo)
		{
			ExpansionDeadDropItemData newCargo = new ExpansionDeadDropItemData();
			newCargo.CopyFrom(carg);
			cargo.Insert(newCargo);
		}
	}

	// Serialize for RPC
	void OnSend(ParamsWriteContext ctx)
	{
		ctx.Write(type);
		ctx.Write(health);
		ctx.Write(quantity);
		ctx.Write(liquidType);
		ctx.Write(ammo);
		ctx.Write(skinIndex);
		ctx.Write(wasInHands);
		ctx.Write(slotName);

		ctx.Write(attachments.Count());
		foreach (ExpansionDeadDropItemData att : attachments)
			ctx.Write(att);

		ctx.Write(cargo.Count());
		foreach (ExpansionDeadDropItemData item : cargo)
			ctx.Write(item);
	}

	// Deserialize from RPC
	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(type)) return false;
		if (!ctx.Read(health)) return false;
		if (!ctx.Read(quantity)) return false;
		if (!ctx.Read(liquidType)) return false;
		if (!ctx.Read(ammo)) return false;
		if (!ctx.Read(skinIndex)) return false;
		if (!ctx.Read(wasInHands)) return false;
		if (!ctx.Read(slotName)) return false;

		int attachmentCount;
		if (!ctx.Read(attachmentCount)) return false;
		attachments = new array<ref ExpansionDeadDropItemData>();
		for (int i = 0; i < attachmentCount; i++)
		{
			ExpansionDeadDropItemData att;
			if (!ctx.Read(att)) return false;
			attachments.Insert(att);
		}

		int cargoCount;
		if (!ctx.Read(cargoCount)) return false;
		cargo = new array<ref ExpansionDeadDropItemData>();
		for (int j = 0; j < cargoCount; j++)
		{
			ExpansionDeadDropItemData item;
			if (!ctx.Read(item)) return false;
			cargo.Insert(item);
		}

		return true;
	}
}
