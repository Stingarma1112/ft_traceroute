#include "../include/ft_traceroute.h"

uint16_t calculate_checksum(void *data, int len) {

	uint16_t *ptr = (uint16_t *)data;
	uint32_t sum = 0;

	while (len > 1) {
		sum += *ptr++;
		len -= 2;
	}
	if (len == 1) {
		sum += *(uint8_t *)ptr << 8;
	}
	while (sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}
	return (~sum);
}

int ft_strcmp(const char *s1, const char *s2) {
	int i = 0;
	while (s1[i] && s2[i]) {
		if (s1[i] != s2[i]) {
			return s1[i] - s2[i];
		}
		i++;
	}
	return s1[i] - s2[i];
}

void	*ft_memset(void *s, int c, size_t n) {
	size_t	i;

	i = 0;
	while (i < n) {
		((unsigned char *) s)[i] = c;
		i++;
	}
	return (s);
}

void	ft_bzero(void *s, size_t n) {
	ft_memset(s, 0, n);
}

void	*ft_memcpy(void *dest, const void *src, size_t n) {
	size_t	i;

	i = 0;
	if (!dest && !src)
		return (0);
	while (i < n) {
		((char *)dest)[i] = ((char *)src)[i];
		i++;
	}	
	return ((void *)dest);
}

size_t	ft_strcpy(char *dst, const char *src)
{
	size_t	i;

	i = 0;
	while (src[i])
	{
		dst[i] = src[i];
		i++;
	}
	dst[i] = '\0';
	return (i);
}