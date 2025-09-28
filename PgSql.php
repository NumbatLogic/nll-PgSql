<?php
	class PgSql_Config extends Project_Config
	{	
		public function __construct($sAction)
		{
			parent::__construct($sAction);
			$this->m_xFileArray = ProjectGen_ParseDirectory(dirname(__FILE__), "/\.h$|\.c$|\.hpp$|\.cpp$/");

			$this->m_xFileArray = array_merge(
				ProjectGen_ParseDirectory(dirname(__FILE__), "/\.h$|\.c$|\.hpp$|\.cpp$/"),
				//ProjectGen_ParseDirectory(dirname(__FILE__) . "/../../Transpiled", "/\.hpp$|\.cpp$/")
			);
		}

		public function GetName() { return "PgSql"; }
		public function GetKind() { return KIND_STATIC_LIBRARY; }
		public function GetBaseDirectory() { return dirname(__FILE__); }

		public function GetIncludeDirectoryArray($sConfiguration, $sArchitecture)
		{
			$sArray = array(
			//	"../LangShared"
			//	"../ThirdParty",
			//	"../Package",
			//	"../Engine",
			);

			/*if ($this->m_sPlatform == PLATFORM_WINDOWS)
			{
				$sArray[] = "../../../Library/Windows/GLEW/include";
				$sArray[] = "../../../Library/Windows/GLFW/include";
				$sArray[] = "../../../Library/Windows/libcurl/include";
			}*/

			return $sArray;
		}

		public function GetDependancyArray()
		{
			$sArray = array(
			//	"LangShared",
			//	"ThirdParty",
			//	"Package",
			//	"Engine",
			//	"Core",
			);

			/*if ($this->m_sAction == ACTION_CMAKE)
			{
				$sArray = array_merge($sArray, array(
					"X11",
					"m",
				));
			}*/

			return $sArray;
		}
	}
?>
