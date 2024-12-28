#include <boost/math/distributions/chi_squared.hpp>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <cmath>
using namespace std;

int word_count = 99171;
int m = 65536;

int str_len(string s)
{
	return s.length() % m;
}

int fst_char(string s)
{
	return uint8_t(s[0]) % m;	
}

int sum_char(string s)
{
	int sum = 0;
	for(int i = 0; i < s.length(); ++i)   
	{
		sum += uint8_t(s[i]);
	}

	return sum % m;
}

int base256(string s)
{
	int remainder_m = 65413;
	int h = 0;
	for(int i = 0; i < s.length(); ++i)
	{
		h = (256 * h + uint8_t(s[i])) % remainder_m;
	}
	return h;
}

int multi_hash(string s)
{
	double A = (sqrt(5)-1) / 2;
	double h = 0.0;
	
	for(int i = 0; i < s.length(); ++i)
	{
		h = std::fmod((256 * h + uint8_t(s[i])) * A, 1) * m;
	}
	return std::floor(h);
}

double pearsons(const vector<int>& hash, int m)
{
	double expected = double(word_count) / m;
	double c2 = 0;
	
	for(int i = 0; i < hash.size(); ++i)
	{
		c2 += ((expected - hash[i])* (expected - hash[i])) / expected;
	}
	return c2;
}

void p_function(double c2, int dof)
{
	boost::math::chi_squared c2d(dof);
	double p = boost::math::cdf(c2d, c2);
	cout << p << endl;
}

void h_function(vector<int> distro)
{	
	int bucket_number = 16;
	vector<int> bucket(bucket_number, 0);
	int bucket_div = distro.size() / bucket_number;
	int height = 20;

	//iterates over each div of the "degrees of freedom"
	for(int i = 0; i < distro.size(); i += bucket_div)
	{
		int bucket_size = 0;
		//for each div it gives the number of indexes with a collision
		for(int j = i; j < (i+bucket_div); j++)
		{
			if(distro[j] > 0)
			{
			  bucket_size++;
		    }
		}
		//make each normalized bucket a specific number to compare
		bucket[i / bucket_div] = bucket_size;
	}
	
	int max_elem = 0;
	for(int i = 0; i < bucket.size(); ++i)
	{
		if(bucket[i] > max_elem)
		{
			max_elem = bucket[i];
		}
	}

	//storing height of each column
	vector<int> normalized(bucket.size(), 0);
	for(int i = 0; i < bucket.size(); ++i)
	{
		normalized[i] = std::floor((double(bucket[i]) / double(max_elem)) * height);
	}

	for(int y = height; y > 0; --y)
	{
		cout << "|";
		for(int x = 0; x < normalized.size(); ++x)
		{
			if(normalized[x] >= y)
			{
				cout << " # ";
			}
			else
			{
				cout << "   ";
			}			
		}
		cout << "|";
		cout << endl;
	}
	cout << " ------------------------------------------------" << endl;
	cout << "  0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 " << endl;
}

int main()
{
	vector<int> vec_len(m,0);
	vector<int> vec_char(m, 0); 
	vector<int> vec_sum(m, 0);
	vector<int> vec_256(65413, 0);
	vector<int> vec_multi(m, 0);
	
	ifstream file("words");

	string word;
	//iterating through each word and making a hash
	while(file >> word)
	{
		//saving hash of string to variable
		int hashes1 = str_len(word);
		int hashes2 = fst_char(word);
		int hashes3 = sum_char(word);
		int hashes4 = base256(word);
		int hashes5 = multi_hash(word);
		
		vec_len[hashes1]++;
		vec_char[hashes2]++;
		vec_sum[hashes3]++;
		vec_256[hashes4]++;
		vec_multi[hashes5]++;
	}

	double c2a = pearsons(vec_len, m);
	double c2b = pearsons(vec_char, m);
	double c2c = pearsons(vec_sum, m);
	double c2d = pearsons(vec_256, 65413);
	double c2e = pearsons(vec_multi, m);

	cout << "String length hash function p-value is: " << endl;
	p_function(c2a, m - 1);
	cout << "First character hash function p-value is: " << endl;
	p_function(c2b, m - 1);
	cout << "Sum of characters hash function p-value is: " << endl;
	p_function(c2c, m - 1);
	cout << "Remainder hash function p-value is: " << endl;
	p_function(c2d, 65413 - 1);
	cout << "Multiplicative hash function p-value is: " << endl;
	p_function(c2e, m - 1);

	cout << "String length hash" << endl;
	h_function(vec_len);

	cout << "First char hash" << endl;
	h_function(vec_char);
	
	cout << "Sum hash" << endl;
	h_function(vec_sum);	
	
	cout << "Remainder hash" << endl;
	h_function(vec_256);	
	
	cout << "Multiplicative hash" << endl;
	h_function(vec_multi);
	
	
}

