<?php
	class PgSql_Config extends Project_Config
	{	
		public function __construct($sAction)
		{
			parent::__construct($sAction);
			$this->m_xFileArray = ProjectGen_ParseDirectory(dirname(__FILE__), ProjectGen_GetSourceRegex($sAction));
		}

		public function GetName() { return "PgSql"; }
		public function GetKind() { return KIND_STATIC_LIBRARY; }
		public function GetBaseDirectory() { return dirname(__FILE__); }

		public function GetIncludeDirectoryArray($sConfiguration, $sArchitecture)
		{
			$sArray = array();
			if (is_dir("/usr/include/postgresql"))
				$sArray[] = "/usr/include/postgresql";
			else
				$sArray[] = "/usr/include";
			return $sArray;
		}

		public function GetDependancyArray()
		{
			if (ProjectGen_ActionIsCS($this->m_sAction))
				return array("Npgsql_10.0.2");

			return array(
				"-lpq",
			);
		}
	}
?>
