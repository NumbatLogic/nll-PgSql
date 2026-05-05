<?php
	include_once dirname(__FILE__) . "/../../ProjectGen/ProjectGen.php";
	include_once dirname(__FILE__) . "/../../LangShared/LangShared.php";
	include_once dirname(__FILE__) . "/PgSql/PgSql.php";
	include_once dirname(__FILE__) . "/DatabaseTestUtils/DatabaseTestUtils.php";
	include_once dirname(__FILE__) . "/PgSqlTest/PgSqlTest.php";

	class PgSql_Solution_Config extends Solution_Config
	{
		public function __construct($sAction)
		{
			parent::__construct($sAction);

			if ($sAction !== "esp_idf")
			{
				$this->m_pProjectArray[] = new LangShared_Config($sAction, dirname(__FILE__) . "/LangShared.package-list");
				$this->m_pProjectArray[] = new PgSql_Config($sAction);
				$this->m_pProjectArray[] = new DatabaseTestUtils_Config($sAction);
				$this->m_pProjectArray[] = new PgSqlTest_Config($sAction);
			}
		}

		public function GetName() { return "PgSql"; }
	}

	ProjectGen(new PgSql_Solution_Config(ProjectGen_GetAction()));
?>
