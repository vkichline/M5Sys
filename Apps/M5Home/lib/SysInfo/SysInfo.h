#pragma once

#define SYSINFO_BUFFER_SIZE   256
#define SYSINFO_PAGE_1_INDENT 120
#define SYSINFO_PAGE_2_INDENT 140

class SysInfo {
  public:
    SysInfo();
    ~SysInfo();
    void  display_info();
    void  page1();
    void  page2();
    void  page3();
  protected:
    void  print_label(const char* label);
    void  print_value(const char* format, ...);
    void  dispatch();

    char* storage;  // Saves first half of a formatted line for second half to print.
    int   indent;
    int   page;
    int   num_pages;
};
