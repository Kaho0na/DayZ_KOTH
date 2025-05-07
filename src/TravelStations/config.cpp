class CfgPatches
{
	class TravelStations_Script
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"DZ_Scripts",
			"DayZExpansion_Core_Scripts"
		};
	};
};
class CfgMods
{
	class TravelStations
	{
		dir="TravelStations";
		picture="";
		action="";
		hideName=1;
		hidePicture=1;
		name="TravelStations";
		credits="Kahoona";
		author="Kahoona";
		authorID="0";
		version="1.1";
		extra=0;
		type="mod";
		dependencies[]=
		{
			"Game",
			"World",
			"Mission"
		};
		class defs
		{
			class gameScriptModule
			{
				value="";
				files[]=
				{
					"TravelStations/scripts/Common",
					"TravelStations/scripts/3_Game"
				};
			};
			class worldScriptModule
			{
				value="";
				files[]=
				{
					"TravelStations/scripts/Common",
					"TravelStations/scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value="";
				files[]=
				{
					"TravelStations/scripts/Common",
					"TravelStations/scripts/5_Mission"
				};
			};
		};
	};
};
