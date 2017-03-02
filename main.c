//
//  main.c
//  memusage
//
//  Created by Alexey Zakharov on 3/2/17.
//  Copyright © 2017 Alexey Zakharov. All rights reserved.
//

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>

#include <mach/mach.h>

long megabytes(uint64_t v)
{
	return v / (1024 * 1024);
}

int main(int argc, const char * argv[])
{
	long page_size = 0;
	uint64_t total_swap_size = 0;
	uint64_t used_swap_size = 0;
	uint64_t total_physical_memory = 0;
	uint64_t used_physical_memory = 0;
	
	int mib[CTL_MAXNAME];
	size_t size;
	struct xsw_usage swap_usage;
	
	// Get page size
	page_size = sysconf(_SC_PAGESIZE);
	
	// Get swap usage
	mib[0] = CTL_VM; mib[1] = VM_SWAPUSAGE;
	size = sizeof(swap_usage);
	if (sysctl(mib, 2, &swap_usage, &size, NULL, 0) != 0)
	{
		fprintf(stderr, "sysctl(VM_SWAPUSAGE): %s\n", strerror(errno));
		return -1;
	}
	total_swap_size = swap_usage.xsu_total;
	used_swap_size = swap_usage.xsu_used;
	
	// Get physical memory size
	mib[0] = CTL_HW; mib[1] = HW_MEMSIZE;
	size = sizeof(total_physical_memory);
	if (sysctl(mib, 2, &total_physical_memory, &size, NULL, 0) != 0)
	{
		fprintf(stderr, "sysctl(HW_MEMSIZE): %s\n", strerror(errno));
		return -1;
	}
	
	// Get used physical memory
	mach_msg_type_number_t vm_stats_size = HOST_VM_INFO64_COUNT;
	vm_statistics64_data_t vm_stats;
	if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vm_stats, &vm_stats_size) != KERN_SUCCESS)
	{
		fprintf(stderr, "host_statistics64(): %s\n", strerror(errno));
		return -1;
	}
	// Used memory is a sum of anonymous pages used by apps that can't be purged and pages reserved by system
	used_physical_memory = (vm_stats.internal_page_count - vm_stats.purgeable_count + vm_stats.wire_count) * page_size;
	
	// Print stats
	printf("Mem usage:\n");
	printf("\tPage size: %ld\n", page_size);
	printf("\tPhysical memory: %ldMB of %ldMB\n", megabytes(used_physical_memory), megabytes(total_physical_memory));
	printf("\tSwap memory: %ldMB of %ldMB\n", megabytes(used_swap_size), megabytes(total_swap_size));
	printf("Total: %ldMB of %ldMB\n", megabytes(used_physical_memory + used_swap_size), megabytes(total_physical_memory + total_swap_size));

	return 0;
}
