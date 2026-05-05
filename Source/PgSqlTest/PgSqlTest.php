<?php
	class PgSqlTest_Config extends Project_Config
	{
		public function __construct($sAction)
		{
			parent::__construct($sAction);

			$this->m_xFileArray = array_merge(
				ProjectGen_ParseDirectory(dirname(__FILE__), ProjectGen_GetSourceRegex($sAction)),
				ProjectGen_ParseDirectory(dirname(__FILE__) . "/../../Transpiled/PgSqlTest", ProjectGen_GetSourceRegex($sAction))
			);
		}

		public function GetName() { return "PgSqlTest"; }
		public function GetKind() { return KIND_CONSOLE_APP; }
		public function GetBaseDirectory() { return dirname(__FILE__); }

		public function GetDependancyArray()
		{
			return array(
				"DatabaseTestUtils",
				"PgSql",
				"LangShared",
			);
		}
	}
?>
