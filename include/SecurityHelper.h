#pragma once

//**************************************************************
// Includes
//**************************************************************

#include <string>

//**************************************************************
// Global function prototypes
//**************************************************************

/*!
 * This function executes a command after validating it for security.
 *
 * The validation process checks for dangerous characters or patterns
 * that could lead to command injection vulnerabilities.
 *
 * @param command The command to be executed.
 * @return true if the command was executed successfully, false otherwise.
 */
extern bool ExecuteCommand(const std::string &command);