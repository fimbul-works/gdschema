extends TestSuite

func _init() -> void:
	icon = "🧪"

func test_dynamic_registration_and_execution() -> void:
	var dynamic_suite = TestSuite.new()
	dynamic_suite.name = "DynamicSubSuite"
	
	var state := {"executed": false}
	dynamic_suite.add_test("dynamic_test_1", func(suite: TestSuite):
		suite.expect(true, "Dynamic test condition should be true")
		suite.expect_equal(2 + 2, 4, "Math should work")
		state.executed = true
	)
	
	var test_names = dynamic_suite.get_test_names()
	expect_equal(test_names.size(), 1, "Should have 1 dynamic test")
	expect(test_names.has("dynamic_test_1"), "Should contain dynamic_test_1")
	
	# Execute dynamic test
	await dynamic_suite.run_test("dynamic_test_1")
	expect(state.executed, "Dynamic test should have executed")
	
	var results = dynamic_suite.get_test_results()
	expect(results.has("dynamic_test_1"), "Results should contain dynamic_test_1")
	expect(results["dynamic_test_1"].passed, "Dynamic test should pass")
	expect_equal(results["dynamic_test_1"].expectation_count, 2, "Should record 2 expectations")
	
	dynamic_suite.free()

func test_dynamic_duplicate_name_prevention() -> void:
	var dynamic_suite = TestSuite.new()
	dynamic_suite.name = "DuplicateTestSubSuite"
	
	dynamic_suite.add_test("custom_test", func(suite: TestSuite): pass)
	
	# Attempt adding with the exact same name
	dynamic_suite.add_test("custom_test", func(suite: TestSuite): pass)
	
	var test_names = dynamic_suite.get_test_names()
	expect_equal(test_names.size(), 1, "Duplicate test should not be added")
	
	dynamic_suite.free()
