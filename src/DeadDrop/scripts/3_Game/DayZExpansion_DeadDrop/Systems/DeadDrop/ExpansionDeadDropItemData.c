/**
 * ExpansionDeadDropItem.c
 *
 * This class defines the structure and data handling
 * for individual items stored in a DeadDrop record.
 */

class DeadDropItem
{
	string type;
	float health;
	float quantity;
	string liquidType;
	int ammo;
	int skinIndex;
	bool wasInHands;
	string slotName;

	ref array<ref DeadDropItem> attachments;
	ref array<ref DeadDropItem> cargo;

	// Constructor
	void DeadDropItem()
	{
		attachments = new array<ref DeadDropItem>();
		cargo = new array<ref DeadDropItem>();
	}

	// Deep copy
	void CopyFrom(DeadDropItem item)
	{
		type = item.type;
		health = item.health;
		quantity = item.quantity;
		liquidType = item.liquidType;
		ammo = item.ammo;
		skinIndex = item.skinIndex;
		wasInHands = item.wasInHands;
		slotName = item.slotName;

		attachments = new array<ref DeadDropItem>();
		foreach (DeadDropItem att : item.attachments)
		{
			DeadDropItem newAtt = new DeadDropItem();
			newAtt.CopyFrom(att);
			attachments.Insert(newAtt);
		}

		cargo = new array<ref DeadDropItem>();
		foreach (DeadDropItem carg : item.cargo)
		{
			DeadDropItem newCargo = new DeadDropItem();
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
		foreach (DeadDropItem att : attachments)
			ctx.Write(att);

		ctx.Write(cargo.Count());
		foreach (DeadDropItem item : cargo)
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
		attachments = new array<ref DeadDropItem>();
		for (int i = 0; i < attachmentCount; i++)
		{
			DeadDropItem att;
			if (!ctx.Read(att)) return false;
			attachments.Insert(att);
		}

		int cargoCount;
		if (!ctx.Read(cargoCount)) return false;
		cargo = new array<ref DeadDropItem>();
		for (int j = 0; j < cargoCount; j++)
		{
			DeadDropItem item;
			if (!ctx.Read(item)) return false;
			cargo.Insert(item);
		}

		return true;
	}
}
