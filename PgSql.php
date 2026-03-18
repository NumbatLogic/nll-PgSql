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
			$sArray = array();
			// PostgreSQL (libpq) include path for libpq-fe.h
			// Common locations: Debian/Ubuntu use /usr/include/postgresql; others may use /usr/include
			if (is_dir("/usr/include/postgresql"))
				$sArray[] = "/usr/include/postgresql";
			else
				$sArray[] = "/usr/include";
			// LangShared InternalString for m_lastError
			$sArray[] = dirname(__FILE__) . "/../LangShared/InternalString/CPP";
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
