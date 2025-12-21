#include "Printer.h"
#include <iostream>


namespace EnGl
{
	void Printer::Print(const glm::vec3& val)
	{
		std::cout << val.x << " " << val.y << " " << val.z << '\n';
	}

	void Printer::Print(const glm::mat4& val)
	{
		std::cout << val[0][0] << " " << val[0][1] << " " << val[0][2] << " " << val[0][3] << '\n';
		std::cout << val[1][0] << " " << val[1][1] << " " << val[1][2] << " " << val[1][3] << '\n';
		std::cout << val[2][0] << " " << val[2][1] << " " << val[2][2] << " " << val[2][3] << '\n';
		std::cout << val[3][0] << " " << val[3][1] << " " << val[3][2] << " " << val[3][3] << '\n';
	}

	void Printer::Print(const glm::mat3x4& val)
	{
		std::cout << val[0][0] << " " << val[0][1] << " " << val[0][2] << " " << val[0][3] << '\n';
		std::cout << val[1][0] << " " << val[1][1] << " " << val[1][2] << " " << val[1][3] << '\n';
		std::cout << val[2][0] << " " << val[2][1] << " " << val[2][2] << " " << val[2][3] << '\n';
	}
}