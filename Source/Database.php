<?php
	include_once dirname(__FILE__) . "/../../ProjectGen/ProjectGen.php";
	include_once dirname(__FILE__) . "/../../LangShared/LangShared.php";
	include_once dirname(__FILE__) . "/Database/Database.php";
	include_once dirname(__FILE__) . "/DatabaseTestUtils/DatabaseTestUtils.php";
	include_once dirname(__FILE__) . "/DatabaseTest/DatabaseTest.php";

	class Database_Solution_Config extends Solution_Config
	{
		public function __construct($sAction)
		{
			parent::__construct($sAction);

			if ($sAction !== "esp_idf")
			{
				$this->m_pProjectArray[] = new LangShared_Config($sAction, dirname(__FILE__) . "/LangShared.package-list");
				$this->m_pProjectArray[] = new Database_Config($sAction);
				$this->m_pProjectArray[] = new DatabaseTestUtils_Config($sAction);
				$this->m_pProjectArray[] = new DatabaseTest_Config($sAction);
			}
		}

		public function GetName() { return "Database"; }
	}

	ProjectGen(new Database_Solution_Config(ProjectGen_GetAction()));
?>