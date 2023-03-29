#pragma once
/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <sstream>
#include <string>
#include <unordered_set>
#include "CrossPlatform.h"

namespace OpenXcom
{

/**
 * Defines the various severity levels of
 * information logged by the game.
 */
enum SeverityLevel
{
	LOG_FATAL,		/**< Something horrible has happened and the game is going to die! */
	LOG_ERROR,		/**< Something bad happened but we can still move on. */
	LOG_WARNING,	/**< Something weird happened, nothing special but it's good to know. */
	LOG_INFO,		/**< Useful information for users/developers to help debug and figure stuff out. */
	LOG_DEBUG,		/**< Purely test stuff to help developers implement, not really relevant to users. */
	LOG_VERBOSE,	/**< Extra details that even developers won't really need 90% of the time. */

	LOG_UNCENSORED  /**< Makes sure everything makes it into log buffer until there's a logfile set up */
};

/**
 * A basic logging and debugging class, prints output to stdout/files.
 * @note Wasn't really satisfied with any of the libraries around
 * so I rolled my own. Based on http://www.drdobbs.com/cpp/logging-in-c/201804215
 */
class Logger
{
public:
	Logger() : _level(LOG_INFO) { };

	// no copy or moves.
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	/// When this class is destroyed, logging happens.
	 ~Logger() { CrossPlatform::log(_level, _message); };

	/// Return access to the ostringstream for logging.
	std::ostringstream& get(SeverityLevel level = LOG_INFO)
	{
		_level = level;
		return _message;
	};

	/// Return access to the ostringstream for logging.
	std::ostringstream& get(std::string key, SeverityLevel level = LOG_INFO)
	{
		_level = level;
		_seenMessages.insert(key);
		return _message;
	};

	/// Determines if this message is a unique message or not.
	/// TODO: switch to contains in C++20.
	static bool isUnquieMessage(const std::string& key) { return _seenMessages.count(key) == 0; } 

	/// Resets seen messages. Call after restart.
	static void resetSeenMessages() { _seenMessages.clear(); }

	/// Set the logging level.
	static SeverityLevel& reportingLevel() {
		static SeverityLevel reportingLevel = LOG_UNCENSORED;
		return reportingLevel;
	};

	/// Translates the logging level to a string.
	static const std::string& toString(int level) {
		static const std::string buffer[] = { "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "VERB", "ALL" };
		return buffer[level];
	};

private:
	SeverityLevel _level;
	std::ostringstream _message;
	inline static std::unordered_set<std::string> _seenMessages{};
};

// these macros return an ostringstream to write log messages to. When the object passes out of scope, it is destroyed and the message is logged.
#define Log(level)          if (level > Logger::reportingLevel()) { } else Logger().get(level)
#define LogOnce(key, level) if (level > Logger::reportingLevel() || Logger::isUnquieMessage(key)) { } else Logger().get(key, level)

}
