#include "../../includes/HttpRequest.hpp"

int	HttpRequest::get_body(std::fstream *inFile, std::fstream *outFile)
{
	std::string line;
	size_t	totalBodySize = 0;
	// std::cout << "max body size " << serverblock.max_body_size << "\n";
	while (true)
	{
		if (!std::getline(*inFile, line))
			return (ERROR_400); // unexpected EOF

		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		else
			return (ERROR_400);

		std::string	sizePart = line.substr(0, line.find(';')); // ignore extension
		if (sizePart.empty()
			|| sizePart.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos)
			return (ERROR_400);


		std::stringstream hexStream(sizePart);
		size_t chunkSize = 0;
		hexStream >> std::hex >> chunkSize;
		if (hexStream.fail())
			return (ERROR_400);


		if (chunkSize == 0) // 0\r\n
		{
			if (!std::getline(*inFile, line)) // consume final \r\n
				return (ERROR_400);
			if (line != "\r")
				return (ERROR_400);
			return (PARSE_OK);
		}


		if (chunkSize > serverblock.max_body_size || totalBodySize > serverblock.max_body_size - chunkSize)
			return (ERROR_413);
		totalBodySize += chunkSize;


		std::string chunk(chunkSize, '\0');
		inFile->read(&chunk[0], chunkSize);
		if ((size_t)(*inFile).gcount() != chunkSize)
			return (ERROR_400);
		*outFile << chunk << "\r\n";
		char	cr, lf;
		if (!inFile->get(cr) || !inFile->get(lf))
			return (ERROR_400);
		if (cr != '\r' || lf != '\n')
			return (ERROR_400);
	}
	// example:
	// 3\r\n
	// wiki\r\n
	// 5\r\n
	// pedia\r\n
	// 0\r\n
	// \r\n
	// decoded body = wikipedia
	return (PARSE_OK);
}

int  HttpRequest::parse_body(std::fstream *inFile, std::fstream *outFile)
{
    return (get_body(inFile, outFile));
}
