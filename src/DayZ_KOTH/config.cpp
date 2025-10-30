class CfgPatches
{
	class KOTH_Scripts
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
	class DayZ_KOTH
	{
		dir="DayZ_KOTH";
		picture="DayZ_KOTH/logo.paa";
		action="";
		hideName=0;
		hidePicture=0;
		name="DayZ King of the Hill";
		credits="Kahoona";
		author="Kahoona";
		authorID="76561199197369229";
		version="1.0.0";
		overview="King of the Hill faction warfare mod with shop system, vehicles, and territory control";
		tooltip="Kahoona - King of the Hill Mod";
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
					"DayZ_KOTH/scripts/Common",
					"DayZ_KOTH/scripts/3_Game"
				};
			};
			class worldScriptModule
			{
				value="";
				files[]=
				{
					"DayZ_KOTH/scripts/Common",
					"DayZ_KOTH/scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value="";
				files[]=
				{
					"DayZ_KOTH/scripts/Common",
					"DayZ_KOTH/scripts/5_Mission"
				};
			};
		};
	};
};