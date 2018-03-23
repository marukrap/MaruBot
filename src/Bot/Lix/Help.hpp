#pragma once

constexpr int even(int x)
{
	return x - (x & 1);
}

constexpr int positiveMod(int nr, int modulo)
{
	if (modulo <= 0)
		return 0;

	const int normalMod = nr % modulo;

	if (normalMod >= 0)
		return normalMod;
	else
		return normalMod + modulo;
}

constexpr int roundInt(float f)
{
	return static_cast<int>(f + 0.5f);
}

constexpr int roundInt(double d)
{
	return static_cast<int>(d + 0.5);
}
