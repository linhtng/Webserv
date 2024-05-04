/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vvagapov <vvagapov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/16 13:08:02 by nlonka            #+#    #+#             */
/*   Updated: 2024/05/04 21:33:09 by vvagapov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

const std::string Logger::_toString(size_t num)
{
	std::stringstream tmp;

	tmp << num;
	return tmp.str();
}

void Logger::initLogger()
{
	size_t i;

	if (!LOG_TO_FILE)
		return;
	for (i = 1; i != 100; i++)
	{
		if (access(LOG_FILE(_toString(i)).c_str(), F_OK))
			break;
	}
}

const std::string Logger::_createTimeStamp()
{
	std::stringstream tmp;
	char buffer[100];
	time_t now = time(nullptr);

	tmp << "[";
	strftime(buffer, 100, "%Y-%m-%d %X", localtime(&now));
	tmp << buffer << "]";
	return tmp.str();
}

const std::string Logger::_levelToString(e_log_level level)
{
	std::string str;

	switch (level)
	{
	case INFO:
		str = "INFO";
		break;
	case DEBUG:
		str = "DEBUG";
		break;
	case ERROR:
		str = "ERROR";
	}
	return str;
}

void Logger::log(e_log_level level, const char *type, const char *format, ...)
{
	std::stringstream message;
	char buffer[LOG_BUF_SIZE];
	va_list args;

	va_start(args, format);
	vsnprintf(buffer, LOG_BUF_SIZE, format, args);
	va_end(args);

	message << type;
	message << _createTimeStamp();
	message << " [" << _levelToString(level) << "]  ";
	message << buffer << RESET << std::endl;
	if (LOG_TO_STDERR)
		std::cerr << message.str();
}
