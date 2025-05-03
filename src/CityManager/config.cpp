class CfgPatches
{
	class CityManager_Script
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
	class CityManager
	{
		dir="CityManager";
		picture="";
		action="";
		hideName=1;
		hidePicture=1;
		name="CityManager";
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
					"CityManager/scripts/Common",
					"CityManager/scripts/3_Game"
				};
			};
			class worldScriptModule
			{
				value="";
				files[]=
				{
					"CityManager/scripts/Common",
					"CityManager/scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value="";
				files[]=
				{
					"CityManager/scripts/Common",
					"CityManager/scripts/5_Mission"
				};
			};
		};
	};
};
