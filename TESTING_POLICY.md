# Testing policy

## Test Generation Rules

### Framework
All tests must use Google Test.

### Structure
- Prefer TEST_F for stateful classes
- Use fixtures for shared setup
- One behavior per test
- Use ASSERT_* for preconditions
- Use EXPECT_* for verifications

### Required Coverage Areas
- nullptr handling
- Thread-safety (if applicable)

### Mocking
- Use Google Mock for interfaces
- Prefer StrictMock unless behavior is intentionally flexible
- Avoid over-specified expectations

