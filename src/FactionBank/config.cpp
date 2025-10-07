class CfgPatches
{
	class FactionBank_Scripts
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
	class FactionBank
	{
		dir="FactionBank";
		picture="";
		action="";
		hideName=1;
		hidePicture=1;
		name="FactionBank";
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
					"FactionBank/scripts/Common",
					"FactionBank/scripts/3_Game"
				};
			};
			class worldScriptModule
			{
				value="";
				files[]=
				{
					"FactionBank/scripts/Common",
					"FactionBank/scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value="";
				files[]=
				{
					"FactionBank/scripts/Common",
					"FactionBank/scripts/5_Mission"
				};
			};
		};
	};
};
