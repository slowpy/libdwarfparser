#include "vmiinstance.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdio.h>
#include <inttypes.h>
#include <cassert>
#include <iostream>
#include "vmiexception.h"

#include "helpers.h"

#define IA32E_IS_PAGE_USER_SUPERVISOR(page_info) \
  	(USER_SUPERVISOR(page_info->x86_ia32e.pte_value) && \
 	 USER_SUPERVISOR(page_info->x86_ia32e.pgd_value) && \
	 USER_SUPERVISOR(page_info->x86_ia32e.pdpte_value) && \
	 USER_SUPERVISOR(page_info->x86_ia32e.pml4e_value))

#define IA32E_IS_PAGE_SUPERVISOR(page_info)        (!(IA32E_IS_PAGE_USER_SUPERVISOR(page_info)))

#define PAGE_NX(entry)	        VMI_GET_BIT(entry, 63)
#define IS_PAGE_NX(page_info) \
	(PAGE_NX(page_info->x86_ia32e.pte_value) || \
	 PAGE_NX(page_info->x86_ia32e.pgd_value) || \
	 PAGE_NX(page_info->x86_ia32e.pdpte_value) || \
	 PAGE_NX(page_info->x86_ia32e.pml4e_value))

/* GLIB Macros */

#define GFOREACH(item, list) for(GList *__glist = list; __glist && (item = __glist->data, true); __glist = __glist->next)

VMIInstance *VMIInstance::instance = NULL;

VMIInstance::VMIInstance(std::string name, uint32_t flags){
	
	if(!flags){
		flags = VMI_AUTO | VMI_INIT_COMPLETE;
	}

    /* initialize the libvmi library */
	char * name_i = const_cast<char*>(name.c_str());
    if (vmi_init(&vmi, flags, name_i) == VMI_FAILURE) {
        printf("Failed to init LibVMI library.\n");
        throw VMIException();
    }

    /* init the offset values */
    if (VMI_OS_LINUX != vmi_get_ostype(vmi)) {
        printf("This tool only supports Linux\n");
        throw VMIException();
    }

	instance = this;
	return;
}

VMIInstance::~VMIInstance(){
    /* resume the vm */
    this->resumeVM();

	instance = NULL;

    /* cleanup any memory associated with the LibVMI instance */
    vmi_destroy(vmi);
	return;
}

VMIInstance *VMIInstance::getInstance(){
	return VMIInstance::instance;
}

void VMIInstance::pauseVM(){
    /* pause the vm for consistent memory access */
    if (vmi_pause_vm(vmi) != VMI_SUCCESS) {
        printf("Failed to pause VM\n");
        throw VMIException();
    }
}

void VMIInstance::resumeVM(){
    /* pause the vm for consistent memory access */
    if (vmi_resume_vm(vmi) != VMI_SUCCESS) {
        printf("Failed to resume VM\n");
        throw VMIException();
    }
}

vmi_instance_t VMIInstance::getLibVMIInstance(){
	return vmi;
}

void VMIInstance::printKernelPages(){

    addr_t init_dtb = vmi_pid_to_dtb(vmi, 1);

    printf("Init dtb is at %p\n", (void*) init_dtb);

    GSList* pages = vmi_get_va_pages(vmi, init_dtb);

    if(pages != NULL) printf("Got Pages\n");

    page_info_t *item;

    for(GSList *__glist = pages; __glist ; __glist = __glist->next){
	    item = (page_info_t*) __glist->data;
	    if (item->x86_ia32e.pml4e_value == 0){
            printf("l4_v is null\n");
    	}else {
			//if(ENTRY_PRESENT(item->x86_ia32e.pte_value, VMI_OS_LINUX) && IA32E_IS_PAGE_SUPERVISOR(item)){
			if(ENTRY_PRESENT(item->x86_ia32e.pte_value, VMI_OS_LINUX) && 
					item->vaddr >> 40 != 0x88){
				if(IA32E_IS_PAGE_USER_SUPERVISOR(item)) continue;
		        printf("Page at %16lx : %016lx - %s, %s ", item->vaddr, item->paddr, (IA32E_IS_PAGE_SUPERVISOR(item)) ? "S": "U" , (IS_PAGE_NX(item)) ? "NX" : "X");
				printf("Page size: %i\n", item->size);
				
				//printf("Settings: %i - %i - %i - %i\n", 
				//    USER_SUPERVISOR(item->x86_ia32e.pte_value),
				//    USER_SUPERVISOR(item->x86_ia32e.pgd_value),
				//    USER_SUPERVISOR(item->x86_ia32e.pdpte_value),
				//    USER_SUPERVISOR(item->x86_ia32e.pml4e_value));
				
			}
        }
		free(item);
    }
	free(pages);
}

PageMap VMIInstance::destroyMap(PageMap map){
	for (auto item : map){
		free(item.second);
	}
	map.clear();
	return map;
}

PageMap VMIInstance::getExecutableKernelPages(){
    
	addr_t init_dtb = vmi_pid_to_dtb(vmi, 1);
    GSList* pages = vmi_get_va_pages(vmi, init_dtb);
    assert(pages);

    page_info_t *item;
	PageMap pageMap;

    for(GSList *__glist = pages; __glist ; __glist = __glist->next){
	    item = (page_info_t*) __glist->data;
	    if (item->x86_ia32e.pml4e_value != 0 &&
		    ENTRY_PRESENT(item->x86_ia32e.pte_value, VMI_OS_LINUX) && 
			item->vaddr >> 40 != 0x88 &&
		    IA32E_IS_PAGE_SUPERVISOR(item) &&
			!IS_PAGE_NX(item)){
			pageMap.insert(std::pair<uint64_t,page_info_t *>(item->vaddr, item));
		}else{
			free(item);
		}
    }
	g_slist_free(pages);
	return pageMap;
}

PageMap VMIInstance::getKernelPages(){
    
	addr_t init_dtb = vmi_pid_to_dtb(vmi, 1);
    GSList* pages = vmi_get_va_pages(vmi, init_dtb);
    assert(pages);

    page_info_t *item;
	PageMap pageMap;

    for(GSList *__glist = pages; __glist ; __glist = __glist->next){
	    item = (page_info_t*) __glist->data;
	    if (item->x86_ia32e.pml4e_value != 0 &&
		    ENTRY_PRESENT(item->x86_ia32e.pte_value, VMI_OS_LINUX) && 
			item->vaddr >> 40 != 0x88 &&
		    IA32E_IS_PAGE_SUPERVISOR(item)){
			pageMap.insert(std::pair<uint64_t,page_info_t *>(item->vaddr, item));
		}else{
			free(item);
		}
    }
	g_slist_free(pages);
	return pageMap;
}


uint8_t VMIInstance::read8FromVA(uint64_t va, uint32_t pid){
	uint8_t value = 0;
	vmi_read_8_va(vmi, va, pid, &value);
	return value;
}

uint16_t VMIInstance::read16FromVA(uint64_t va, uint32_t pid){
	uint16_t value = 0;
	vmi_read_16_va(vmi, va, pid, &value);
	return value;
}

uint32_t VMIInstance::read32FromVA(uint64_t va, uint32_t pid){
	uint32_t value = 0;
	vmi_read_32_va(vmi, va, pid, &value);
	return value;
}

uint64_t VMIInstance::read64FromVA(uint64_t va, uint32_t pid){
	uint64_t value = 0;
	vmi_read_64_va(vmi, va, pid, &value);
	return value;
}

std::vector<uint8_t> 
    VMIInstance::readVectorFromVA(uint64_t va, uint64_t len,
                                  uint32_t pid){
	uint8_t* buffer = (uint8_t*) malloc(len);
	vmi_read_va(vmi, va, pid, buffer, len);

	std::vector<uint8_t> result;
	result.insert(result.end(), buffer, buffer + len);
	free(buffer);
	return result;

}

std::string VMIInstance::readStrFromVA(uint64_t va, uint32_t pid){
	char * str = vmi_read_str_va(vmi, va, pid);
	assert(str);
	std::string result = std::string(str);
	delete str;
	return result;
}

bool VMIInstance::isPageSuperuser(page_info_t* page){
    bool ret = IA32E_IS_PAGE_SUPERVISOR(page);
	return ret;
}

bool VMIInstance::isPageExecutable(page_info_t* page){
	bool ret = !IS_PAGE_NX(page);
	return ret;
}

