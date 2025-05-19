class CfgPatches
{
	class DeadDrop_Script
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
	class DeadDrop
	{
		dir="DeadDrop";
		picture="";
		action="";
		hideName=1;
		hidePicture=1;
		name="DeadDrop";
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
					"DeadDrop/scripts/Common",
					"DeadDrop/scripts/3_Game"
				};
			};
			class worldScriptModule
			{
				value="";
				files[]=
				{
					"DeadDrop/scripts/Common",
					"DeadDrop/scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value="";
				files[]=
				{
					"DeadDrop/scripts/Common",
					"DeadDrop/scripts/5_Mission"
				};
			};
		};
	};
};
