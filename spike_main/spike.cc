// See LICENSE for license details.

#include "sim.h"
#include "mmu.h"
#include "remote_bitbang.h"
#include "cachesim.h"
#include "extension.h"
#include <dlfcn.h>
#include <fesvr/option_parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include "../VERSION"

static void help(int exit_code = 1)
{
  fprintf(stderr, "Spike RISC-V ISA Simulator " SPIKE_VERSION "\n\n");
  fprintf(stderr, "usage: spike [host options] <target program> [target options]\n");
  fprintf(stderr, "Host Options:\n");
  fprintf(stderr, "  -p<n>                 Simulate <n> processors [default 1]\n");
  fprintf(stderr, "  -b<n>                 Simulate <n> banks [default 1]. 0 == nprocs %% nbanks \n");
  fprintf(stderr, "  -m<n>                 Provide <n> MiB of target memory [default 2048]\n");
  fprintf(stderr, "  -m<a:m,b:n,...>       Provide memory regions of size m and n bytes\n");
  fprintf(stderr, "                          at base addresses a and b (with 4 KiB alignment)\n");
  fprintf(stderr, "  -d                    Interactive debug mode\n");
  fprintf(stderr, "  -g                    Track histogram of PCs\n");
  fprintf(stderr, "  -l                    Generate a log of execution\n");
  fprintf(stderr, "  -h, --help            Print this help message\n");
  fprintf(stderr, "  -H                    Start halted, allowing a debugger to connect\n");
  fprintf(stderr, "  --pcie-enabled        Start PCIE driver\n");
  fprintf(stderr, "  --output-ddr-id-enabled         Output memery file name with bank id [default false]\n");
  fprintf(stderr, "  --isa=<name>          RISC-V ISA string [default %s]\n", DEFAULT_ISA);
  fprintf(stderr, "  --priv=<m|mu|msu>     RISC-V privilege modes supported [default %s]\n", DEFAULT_PRIV);
  fprintf(stderr, "  --varch=<name>        RISC-V Vector uArch string [default %s]\n", DEFAULT_VARCH);
  fprintf(stderr, "  --pc=<address>        Override ELF entry point\n");
  fprintf(stderr, "  --hartids=<a,b,...>   Explicitly specify hartids, default is 0,1,...\n");
  fprintf(stderr, "  --ic=<S>:<W>:<B>      Instantiate a cache model with S sets,\n");
  fprintf(stderr, "  --dc=<S>:<W>:<B>        W ways, and B-byte blocks (with S and\n");
  fprintf(stderr, "  --l2=<S>:<W>:<B>        B both powers of 2).\n");
  fprintf(stderr, "  --device=<P,B,A>      Attach MMIO plugin device from an --extlib library\n");
  fprintf(stderr, "                          P -- Name of the MMIO plugin\n");
  fprintf(stderr, "                          B -- Base memory address of the device\n");
  fprintf(stderr, "                          A -- String arguments to pass to the plugin\n");
  fprintf(stderr, "                          This flag can be used multiple times.\n");
  fprintf(stderr, "                          The extlib flag for the library must come first.\n");
  fprintf(stderr, "  --log-cache-miss      Generate a log of cache miss\n");
  fprintf(stderr, "  --extension=<name>    Specify RoCC Extension\n");
  fprintf(stderr, "  --extlib=<name>       Shared library to load\n");
  fprintf(stderr, "                        This flag can be used multiple times.\n");
  fprintf(stderr, "  --rbb-port=<port>     Listen on <port> for remote bitbang connection\n");
  fprintf(stderr, "  --dump-dts            Print device tree string and exit\n");
  fprintf(stderr, "  --disable-dtb         Don't write the device tree blob into memory\n");
  fprintf(stderr, "  --bank-id=<n>         Single bank(-b=1):NPU Bank ID [default 0]. Multi bank(-b>1): id of the first bank [default 0].\n");
  fprintf(stderr, "  --board-id=<n>        Indicates the number of boards in a rack [default 0]\n");
  fprintf(stderr, "  --chip-id=<n>         Several chips per board [default 0]\n");
  fprintf(stderr, "  --hwsync-masks=<0xxx,0xxx,>  HWsync masks \n");
  fprintf(stderr, "  --session-id=<n>      HWsync shared memory id [default 0]\n");
  fprintf(stderr, "  --core-mask=<n>         set core mask, bit0-bit31 for core0-core31 [default 0xffffffff all unmask]\n");
  fprintf(stderr, "  --ddr-size=<words>    DDR Memory size [default 0xa00000, 10MB]\n");
  fprintf(stderr, "  --atuini=<path>       Address translation configuration file for virtualization\n");
  fprintf(stderr, "  --kernel=<path>       Load kernel flat image into memory\n");
  fprintf(stderr, "  --initrd=<path>       Load kernel initrd into memory\n");
  fprintf(stderr, "  --bootargs=<args>     Provide custom bootargs for kernel [default: console=hvc0 earlycon=sbi]\n");
  fprintf(stderr, "  --real-time-clint     Increment clint time at real-time rate\n");
  fprintf(stderr, "  --dm-progsize=<words> Progsize for the debug module [default 2]\n");
  fprintf(stderr, "  --dm-sba=<bits>       Debug bus master supports up to "
      "<bits> wide accesses [default 0]\n");
  fprintf(stderr, "  --dm-auth             Debug module requires debugger to authenticate\n");
  fprintf(stderr, "  --dmi-rti=<n>         Number of Run-Test/Idle cycles "
      "required for a DMI access [default 0]\n");
  fprintf(stderr, "  --dm-abstract-rti=<n> Number of Run-Test/Idle cycles "
      "required for an abstract command to execute [default 0]\n");
  fprintf(stderr, "  --dm-no-hasel         Debug module supports hasel\n");
  fprintf(stderr, "  --dm-no-abstract-csr  Debug module won't support abstract to authenticate\n");
  fprintf(stderr, "  --dm-no-halt-groups   Debug module won't support halt groups\n");
  fprintf(stderr, "  --dm-no-impebreak     Debug module won't support implicit ebreak in program buffer\n");

  fprintf(stderr, "BackDoor Options:\n");
  fprintf(stderr, "  --load=<file1,...>    load files into memory\n");
  fprintf(stderr, "                          file name: *@[core_id|ddr|llb.][<start>_<len>].<ext>\n");
  fprintf(stderr, "                          example: aaa@0.bin, bbb@llb.dat, ccc@ddr.dat,\n");
  fprintf(stderr, "                                   aaa@0.0xc0000000_0x100.dat, aaa@ddr.0x0_0x10000.dat\n");
  fprintf(stderr, "  --init-dump=<m1,...>  Dump memory on init\n");
  fprintf(stderr, "  --exit-dump=<m1,...>  Dump memory on exit\n");
  fprintf(stderr, "                          memory range could be: l1, llb, <start>:<len>\n");
  fprintf(stderr, "  --dump-path           Path for files to dump memory [default .]\n");
  fprintf(stderr, "  --sync-timeout=<n>    Timeout ticks for sync throw trap [default 0xffffffff]\n");
  exit(exit_code);
}

static void suggest_help()
{
  fprintf(stderr, "Try 'spike --help' for more information.\n");
  exit(1);
}

static bool check_file_exists(const char *fileName)
{
  std::ifstream infile(fileName);
  return infile.good();
}

static std::ifstream::pos_type get_file_size(const char *filename)
{
  std::ifstream in(filename, std::ios::ate | std::ios::binary);
  return in.tellg();
}

static void read_file_bytes(const char *filename,size_t fileoff,
                            char *read_buf, size_t read_sz)
{
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  in.seekg(fileoff, std::ios::beg);
  in.read(read_buf, read_sz);
}

bool sort_mem_region(const std::pair<reg_t, mem_t*> &a,
                       const std::pair<reg_t, mem_t*> &b)
{
  if (a.first == b.first)
    return (a.second->size() < b.second->size());
  else
    return (a.first < b.first);
}

void merge_overlapping_memory_regions(std::vector<std::pair<reg_t, mem_t*>>& mems)
{
  // check the user specified memory regions and merge the overlapping or
  // eliminate the containing parts
  std::sort(mems.begin(), mems.end(), sort_mem_region);
  reg_t start_page = 0, end_page = 0;
  std::vector<std::pair<reg_t, mem_t*>>::reverse_iterator it = mems.rbegin();
  std::vector<std::pair<reg_t, mem_t*>>::reverse_iterator _it = mems.rbegin();
  for(; it != mems.rend(); ++it) {
    reg_t _start_page = it->first/PGSIZE;
    reg_t _end_page = _start_page + it->second->size()/PGSIZE;
    if (_start_page >= start_page && _end_page <= end_page) {
      // contains
      mems.erase(std::next(it).base());
    }else if ( _start_page < start_page && _end_page > start_page) {
      // overlapping
      _it->first = _start_page;
      if (_end_page > end_page)
        end_page = _end_page;
      mems.erase(std::next(it).base());
    }else {
      _it = it;
      start_page = _start_page;
      end_page = _end_page;
      assert(start_page < end_page);
    }
  }
}

#define DEFAULT_MEMORY_LAYOUT "0x0:0xc0000000"

static std::vector<std::pair<reg_t, mem_t*>> make_mems(const char* arg)
{
  // handle legacy mem argument
  char* p;
  auto mb = strtoull(arg, &p, 0);
  if (*p == 0) {
    reg_t size = reg_t(mb) << 20;
    if (size != (size_t)size)
      throw std::runtime_error("Size would overflow size_t");
    return std::vector<std::pair<reg_t, mem_t*>>(1, std::make_pair(reg_t(DRAM_BASE), new mem_t(size)));
  }

  // handle base/size tuples
  std::vector<std::pair<reg_t, mem_t*>> res;
  while (true) {
    auto base = strtoull(arg, &p, 0);
    if (!*p || *p != ':')
      help();
    auto size = strtoull(p + 1, &p, 0);

    // page-align base and size
    auto base0 = base, size0 = size;
    size += base0 % PGSIZE;
    base -= base0 % PGSIZE;
    if (size % PGSIZE != 0)
      size += PGSIZE - size % PGSIZE;

    if (base + size < base)
      help();

    if (size != size0) {
      fprintf(stderr, "Warning: the memory at  [0x%llX, 0x%llX] has been realigned\n"
                      "to the %ld KiB page size: [0x%llX, 0x%llX]\n",
              base0, base0 + size0 - 1, PGSIZE / 1024, base, base + size - 1);
    }

    res.push_back(std::make_pair(reg_t(base), new mem_t(size)));
    if (!*p)
      break;
    if (*p != ',')
      help();
    arg = p + 1;
  }

  merge_overlapping_memory_regions(res);
  return res;
}

static unsigned long atoul_safe(const char* s)
{
  char* e;
  auto res = strtoul(s, &e, 10);
  if (*e)
    help();
  return res;
}

static unsigned long atoul_nonzero_safe(const char* s)
{
  auto res = atoul_safe(s);
  if (!res)
    help();
  return res;
}

static std::vector<std::string> make_strings(const char* arg)
{
  std::stringstream ss(arg);
  std::string item;
  std::vector<std::string> result;
  while (std::getline(ss, item, ',')) {
    result.push_back(std::move(item));
  }
  return result;
}

int main(int argc, char** argv)
{
  bool debug = false;
  bool halted = false;
  bool histogram = false;
  bool log = false;
  bool dump_dts = false;
  bool dtb_enabled = true;
  bool real_time_clint = false;
  bool pcie_enabled = false;
  bool file_name_with_bank_id = false;
  int nbanks = 1;
  size_t nprocs = 1;
  uint64_t ddr_size = 0x100000000; //4G ddr
  size_t board_id = 0;
  size_t chip_id = 0;
  size_t id_first_bank = 0;
  size_t die_id = 0;
  size_t session_id = 0;
  uint32_t coremask = 0xffffffff;
  uint32_t hwsync_timer_num = 0xffffffff;    /* hs register HS_SYNC_REQ_ THRESH */
  char masks_buf[178]={'\0'};
  char *hwsync_masks = masks_buf;
  const char* atuini = nullptr;         /* 地址转换单元的配置文件 */

  const char* kernel = NULL;
  reg_t kernel_offset, kernel_size;
  size_t initrd_size;
  reg_t initrd_start = 0, initrd_end = 0;
  const char* bootargs = NULL;
  reg_t start_pc = reg_t(-1);
  std::vector<std::pair<reg_t, mem_t*>> mems;
  std::vector<std::pair<reg_t, abstract_device_t*>> plugin_devices;
  std::unique_ptr<icache_sim_t> ic;
  std::unique_ptr<dcache_sim_t> dc;
  std::unique_ptr<cache_sim_t> l2;
  bool log_cache = false;
  bool log_commits = false;
  const char *log_path = nullptr;
  std::function<extension_t*()> extension;
  const char* initrd = NULL;
  const char* isa = DEFAULT_ISA;
  const char* priv = DEFAULT_PRIV;
  const char* varch = DEFAULT_VARCH;
  const char* dtb_file = NULL;
  std::vector<std::string> load_files;
  std::vector<std::string> init_dump;
  std::vector<std::string> exit_dump;
  std::string dump_path = ".";
  uint16_t rbb_port = 0;
  bool use_rbb = false;
  unsigned dmi_rti = 0;
  debug_module_config_t dm_config = {
    .progbufsize = 2,
    .max_bus_master_bits = 0,
    .require_authentication = false,
    .abstract_rti = 0,
    .support_hasel = true,
    .support_abstract_csr_access = true,
    .support_haltgroups = true,
    .support_impebreak = true
  };
  std::vector<int> hartids;

  auto const hartids_parser = [&](const char *s) {
    std::string const str(s);
    std::stringstream stream(str);

    int n;
    while (stream >> n)
    {
      hartids.push_back(n);
      if (stream.peek() == ',') stream.ignore();
    }
  };

  auto const device_parser = [&plugin_devices](const char *s) {
    const std::string str(s);
    std::istringstream stream(str);

    // We are parsing a string like name,base,args.

    // Parse the name, which is simply all of the characters leading up to the
    // first comma. The validity of the plugin name will be checked later.
    std::string name;
    std::getline(stream, name, ',');
    if (name.empty()) {
      throw std::runtime_error("Plugin name is empty.");
    }

    // Parse the base address. First, get all of the characters up to the next
    // comma (or up to the end of the string if there is no comma). Then try to
    // parse that string as an integer according to the rules of strtoull. It
    // could be in decimal, hex, or octal. Fail if we were able to parse a
    // number but there were garbage characters after the valid number. We must
    // consume the entire string between the commas.
    std::string base_str;
    std::getline(stream, base_str, ',');
    if (base_str.empty()) {
      throw std::runtime_error("Device base address is empty.");
    }
    char* end;
    reg_t base = static_cast<reg_t>(strtoull(base_str.c_str(), &end, 0));
    if (end != &*base_str.cend()) {
      throw std::runtime_error("Error parsing device base address.");
    }

    // The remainder of the string is the arguments. We could use getline, but
    // that could ignore newline characters in the arguments. That should be
    // rare and discouraged, but handle it here anyway with this weird in_avail
    // technique. The arguments are optional, so if there were no arguments
    // specified we could end up with an empty string here. That's okay.
    auto avail = stream.rdbuf()->in_avail();
    std::string args(avail, '\0');
    stream.readsome(&args[0], avail);

    plugin_devices.emplace_back(base, new mmio_plugin_device_t(name, args));
  };

  option_parser_t parser;
  parser.help(&suggest_help);
  parser.option('h', "help", 0, [&](const char* s){help(0);});
  parser.option('d', 0, 0, [&](const char* s){debug = true;});
  parser.option('g', 0, 0, [&](const char* s){histogram = true;});
  parser.option('l', 0, 0, [&](const char* s){log = true;});
  parser.option('p', 0, 1, [&](const char* s){nprocs = atoul_nonzero_safe(s);});
  parser.option('b', 0, 1, [&](const char* s){nbanks = atoul_nonzero_safe(s);});
  parser.option('m', 0, 1, [&](const char* s){mems = make_mems(s);});
  parser.option(0, "pcie-enabled", 0, [&](const char *s) { pcie_enabled = true; });
  parser.option(0, "output-ddr-id-enabled", 0, [&](const char *s) { file_name_with_bank_id = true; });
  parser.option(0, "bank-id", 1, [&](const char* s){ id_first_bank = atoi(s);});
  parser.option(0, "die-id", 1, [&](const char* s){ die_id = atoi(s);});
  parser.option(0, "board-id", 1, [&](const char *s) { board_id = atoi(s); });
  parser.option(0, "chip-id", 1, [&](const char *s) { chip_id = atoi(s); });
  parser.option(0, "session-id", 1, [&](const char *s) { session_id = atoi(s); });
  parser.option(0, "core-mask", 1, [&](const char *s) { coremask = strtoull(s, NULL, 0); });
  parser.option(0, "hwsync-masks", 1, [&](const char *s) { hwsync_masks = (char *)s; });
  parser.option(0, "ddr-size", 1, [&](const char *s) { ddr_size = strtoull(s, NULL, 0); });
  // I wanted to use --halted, but for some reason that doesn't work.
  parser.option('H', 0, 0, [&](const char* s){halted = true;});
  parser.option(0, "rbb-port", 1, [&](const char* s){use_rbb = true; rbb_port = atoul_safe(s);});
  parser.option(0, "pc", 1, [&](const char* s){start_pc = strtoull(s, 0, 0);});
  parser.option(0, "hartids", 1, hartids_parser);
  parser.option(0, "ic", 1, [&](const char* s){ic.reset(new icache_sim_t(s));});
  parser.option(0, "dc", 1, [&](const char* s){dc.reset(new dcache_sim_t(s));});
  parser.option(0, "l2", 1, [&](const char* s){l2.reset(cache_sim_t::construct(s, "L2$"));});
  parser.option(0, "log-cache-miss", 0, [&](const char* s){log_cache = true;});
  parser.option(0, "isa", 1, [&](const char* s){isa = s;});
  parser.option(0, "priv", 1, [&](const char* s){priv = s;});
  parser.option(0, "varch", 1, [&](const char* s){varch = s;});
  parser.option(0, "device", 1, device_parser);
  parser.option(0, "extension", 1, [&](const char* s){extension = find_extension(s);});
  parser.option(0, "dump-dts", 0, [&](const char *s){dump_dts = true;});
  parser.option(0, "disable-dtb", 0, [&](const char *s){dtb_enabled = false;});
  parser.option(0, "dtb", 1, [&](const char *s){dtb_file = s;});
  parser.option(0, "atuini", 1, [&](const char* s){atuini = s;});
  parser.option(0, "kernel", 1, [&](const char* s){kernel = s;});
  parser.option(0, "initrd", 1, [&](const char* s){initrd = s;});
  parser.option(0, "bootargs", 1, [&](const char* s){bootargs = s;});
  parser.option(0, "real-time-clint", 0, [&](const char *s){real_time_clint = true;});
  parser.option(0, "extlib", 1, [&](const char *s){
    void *lib = dlopen(s, RTLD_NOW | RTLD_GLOBAL);
    if (lib == NULL) {
      fprintf(stderr, "Unable to load extlib '%s': %s\n", s, dlerror());
      exit(-1);
    }
  });
  parser.option(0, "dm-progsize", 1,
      [&](const char* s){dm_config.progbufsize = atoul_safe(s);});
  parser.option(0, "dm-no-impebreak", 0,
      [&](const char* s){dm_config.support_impebreak = false;});
  parser.option(0, "dm-sba", 1,
      [&](const char* s){dm_config.max_bus_master_bits = atoul_safe(s);});
  parser.option(0, "dm-auth", 0,
      [&](const char* s){dm_config.require_authentication = true;});
  parser.option(0, "dmi-rti", 1,
      [&](const char* s){dmi_rti = atoul_safe(s);});
  parser.option(0, "dm-abstract-rti", 1,
      [&](const char* s){dm_config.abstract_rti = atoul_safe(s);});
  parser.option(0, "dm-no-hasel", 0,
      [&](const char* s){dm_config.support_hasel = false;});
  parser.option(0, "dm-no-abstract-csr", 0,
      [&](const char* s){dm_config.support_abstract_csr_access = false;});
  parser.option(0, "dm-no-halt-groups", 0,
      [&](const char* s){dm_config.support_haltgroups = false;});
  parser.option(0, "log-commits", 0,
                [&](const char* s){log_commits = true;});
  parser.option(0, "log", 1,
                [&](const char* s){log_path = s;});

  /* a backdoor for ncbet
   * load-path is case input path
   * dump-path is memory dump path, for ncbet get result
   */
  parser.option(0, "load", 1, [&](const char* s){load_files = make_strings(s);});
  parser.option(0, "init-dump", 1, [&](const char* s){init_dump = make_strings(s);});
  parser.option(0, "exit-dump", 1, [&](const char* s){exit_dump = make_strings(s);});
  parser.option(0, "dump-path", 1, [&](const char* s){dump_path = s;});
  parser.option(0, "sync-timeout", 1, [&](const char* s){hwsync_timer_num = strtoull(s, NULL, 0); });

  auto argv1 = parser.parse(argv);
  std::vector<std::string> htif_args(argv1, (const char*const*)argv + argc);
  if (ddr_size == 0 && mems.empty())
    mems = make_mems(DEFAULT_MEMORY_LAYOUT);

  if (!*argv1)
    help();

  if (kernel && check_file_exists(kernel)) {
    kernel_size = get_file_size(kernel);
    if (isa[2] == '6' && isa[3] == '4')
      kernel_offset = 0x200000;
    else
      kernel_offset = 0x400000;
    for (auto& m : mems) {
      if (kernel_size && (kernel_offset + kernel_size) < m.second->size()) {
         read_file_bytes(kernel, 0, m.second->contents() + kernel_offset, kernel_size);
         break;
      }
    }
  }

  if (initrd && check_file_exists(initrd)) {
    initrd_size = get_file_size(initrd);
    for (auto& m : mems) {
      if (initrd_size && (initrd_size + 0x1000) < m.second->size()) {
         initrd_end = m.first + m.second->size() - 0x1000;
         initrd_start = initrd_end - initrd_size;
         read_file_bytes(initrd, 0, m.second->contents() + (initrd_start - m.first), initrd_size);
         break;
      }
    }
  }

  if (((int)(nbanks)>(int)(nprocs)) || (0!=nprocs%nbanks)) {
        std::cerr << "error " << "-p" << nprocs
                << " -b" << nbanks << ".\n";
        exit(1);
  }
  sim_t s(isa, priv, varch, nprocs, nbanks, id_first_bank, die_id, (char *)hwsync_masks, hwsync_timer_num, halted, real_time_clint,
      initrd_start, initrd_end, bootargs, start_pc, mems, ddr_size, plugin_devices,
      htif_args, std::move(hartids), dm_config, log_path, dtb_enabled, dtb_file,
      pcie_enabled, file_name_with_bank_id, board_id, chip_id, session_id, coremask, atuini);
  std::unique_ptr<remote_bitbang_t> remote_bitbang((remote_bitbang_t *) NULL);
  std::unique_ptr<jtag_dtm_t> jtag_dtm(
      new jtag_dtm_t(&s.debug_module, dmi_rti));
  if (use_rbb) {
    remote_bitbang.reset(new remote_bitbang_t(rbb_port, &(*jtag_dtm)));
    s.set_remote_bitbang(&(*remote_bitbang));
  }

  if (dump_dts) {
    printf("%s", s.get_dts());
    return 0;
  }

  if (ic && l2) ic->set_miss_handler(&*l2);
  if (dc && l2) dc->set_miss_handler(&*l2);
  if (ic) ic->set_log(log_cache);
  if (dc) dc->set_log(log_cache);
  for (int i = 0; i < (int)nprocs; i++)
  {
    if (ic) s.get_core_by_idxinsim(i)->get_mmu()->register_memtracer(&*ic);
    if (dc) s.get_core_by_idxinsim(i)->get_mmu()->register_memtracer(&*dc);
    if (extension) s.get_core_by_idxinsim(i)->register_extension(extension());
  }

  s.set_debug(debug);
  s.configure_log(log, log_commits);
  s.set_histogram(histogram);

  auto return_code = s.run(load_files, init_dump, exit_dump, dump_path);

  for (auto& mem : mems)
    delete mem.second;

  for (auto& plugin_device : plugin_devices)
    delete plugin_device.second;

  return return_code;
}
