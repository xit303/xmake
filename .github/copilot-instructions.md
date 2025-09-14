---
applyTo: "**"
---
# Project general coding standards

## Naming Conventions
- Use PascalCase for component names, interfaces, and type aliases
- Use camelCase for variables, functions, and methods

## Error Handling
- Use try/catch blocks for async operations
- Implement proper error boundaries in React components
- Always log errors with contextual information

## Commit Messages
- Use conventional commit format: `<type>(<scope>): <subject>`
- Types can include `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`
- Scope is optional but can be used to specify the area of the codebase affected
- Subject should be in imperative mood (e.g., "Add feature" instead of "Added feature")
- Keep the subject line concise (50 characters or less)
- Use the body for detailed explanations if necessary
- Example: `feat(ui): add new button component`

## Testing
- Write unit tests for all components and utility functions
- Use google test for tests; create many, considering data types and variables.
- Ensure tests cover edge cases and error scenarios
- Use descriptive test names that explain the purpose of the test
