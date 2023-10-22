
#include "gravity_compiler.h"
#include "gravity_optionals.h"
#include "gravity_utils.h"
#include "gravity_core.h"
#include "gravity_vm.h"
#include "gravity_opt_env.h"
#include "gravity_macros.h"
#include "gravity_vmmacros.h"

#include <string>

int main_rect(int argc, const char * argv[]);

static bool NativeFunction(gravity_vm *vm, gravity_value_t *args,
                                uint16_t nargs, uint32_t rindex)
{
    printf(" >> NativeFunction called ! nargs = %d, rindex = %d\n", nargs, rindex);
    char buf[1024];
    std::string ret;
    for(int i = 0 ; i < nargs ; ++i){
        gravity_value_dump(vm, args[i], buf, 1024);
        ret += buf;
        ret += "\n";
    }
    printf(" >> nargs = %s\n", ret.data());
    gravity_closure_t* closure = (gravity_closure_t*)gravity_vm_getvalue(vm, "GravityFunction", strlen("GravityFunction")).p;

    gravity_vm_runclosure(vm, closure, VALUE_FROM_NULL, NULL, 0);
    RETURN_VALUE(VALUE_FROM_NULL, rindex);
}

int main(int argc,  const char * argv[])
{
    setbuf(stdout, NULL);
    main_rect(argc, argv);

    gravity_delegate_t delegate = {};
    gravity_compiler_t* compiler = gravity_compiler_create(&delegate);

    const char* source_code =
    "extern var NativeClass\n"
    "\n"
    "func GravityFunction()\n"
    "{\n"
    "    for (var i in 0..<1000)\n"
    "    {\n"
    "        var s = []\n"
    "\n"
    "        for (var j in 0..<10)\n"
    "        {\n"
    "            s.push(j)\n"
    "        }\n"
    "    }\n"
    "}\n"
    "\n"
    "func main()\n"
    "{\n"
    "    NativeClass(1,2,3)\n"
    "}\n";

    gravity_closure_t* closure = gravity_compiler_run(compiler, source_code,
                                        strlen(source_code), 0, true, true);

    gravity_vm* vm = gravity_vm_new(&delegate);
    gravity_compiler_transfer(compiler, vm);
    gravity_compiler_free(compiler);

    //nsvar: number of static vars
    //nivar: number of obj vars
    gravity_class_t* klass = gravity_class_new_pair(NULL, "NativeClass", NULL, 0, 0);
    gravity_class_bind(klass, "init", NEW_CLOSURE_VALUE(NativeFunction));
    gravity_vm_setvalue(vm, "NativeClass", gravity_value_from_object(klass));

    gravity_vm_runmain(vm, closure);

    gravity_vm_free(vm);
    gravity_core_free();

    return EXIT_SUCCESS;
}
