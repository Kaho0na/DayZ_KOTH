class TravelStationLocation
{
	int CityID;
	string StationName;
	int StationID;
	vector Position;
	ref array<vector> TPPositions;
	bool IsLiberated;

	void SetTPPositions(vector Position)
	{
		TPPositions = new array<vector>();
	
		// Original Y stays constant
		float x = Position[0];
		float y = Position[1];
		float z = Position[2];
	
		// Add each calculated offset vector to the array
		TPPositions.Insert(Vector(x + 2, y, z - 2));
		TPPositions.Insert(Vector(x + 4, y, z - 2));
		TPPositions.Insert(Vector(x + 2, y, z - 4));
		TPPositions.Insert(Vector(x + 4, y, z - 4));
	}

	void OnSend(ParamsWriteContext ctx)
	{
		ctx.Write(CityID);
		ctx.Write(StationName);
		ctx.Write(StationID);
		ctx.Write(TPPositions);
		ctx.Write(Position);
		ctx.Write(IsLiberated);
	}

	bool OnReceive(ParamsReadContext ctx)
	{
		if (!ctx.Read(CityID))
		{
			Error(ToString() + "::OnReceive - CityID");
			return false;
		}

		if (!ctx.Read(StationName))
		{
			Error(ToString() + "::OnReceive - StationName");
			return false;
		}

		if (!ctx.Read(StationID))
		{
			Error(ToString() + "::OnReceive - StationID");
			return false;
		}

		if (!ctx.Read(TPPositions))
		{
			Error(ToString() + "::OnReceive - TPPositions");
			return false;
		}

		if (!ctx.Read(Position))
		{
			Error(ToString() + "::OnReceive - TPPositions");
			return false;
		}

		if (!ctx.Read(IsLiberated))
		{
			Error(ToString() + "::OnReceive - TPPositions");
			return false;
		}

		return true;
	}

};