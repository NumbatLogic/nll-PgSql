<?php
	class DatabaseTestUtils_Config extends Project_Config
	{
		public function __construct($sAction)
		{
			parent::__construct($sAction);

			$this->m_xFileArray = array_merge(
				ProjectGen_ParseDirectory(dirname(__FILE__), ProjectGen_GetSourceRegex($sAction)),
				ProjectGen_ParseDirectory(dirname(__FILE__) . "/../../Transpiled/DatabaseTestUtils", ProjectGen_GetSourceRegex($sAction))
			);
		}

		public function GetName() { return "DatabaseTestUtils"; }
		public function GetKind() { return KIND_STATIC_LIBRARY; }
		public function GetBaseDirectory() { return dirname(__FILE__); }

		public function GetDependancyArray()
		{
			return array(
				"Database",
				"LangShared",
			);
		}
	}
?>
