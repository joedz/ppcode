struct stack_t
{
    void    *   sp;
    std::size_t size;

    stack_t() :
        sp( 0), size( 0)
    {}
};

struct fp_t
{
    boost::uint32_t     fc_freg[16];

    fp_t() :
        fc_freg()
    {}
};

struct fcontext_t
{
    boost::uint32_t     fc_greg[11];
    stack_t             fc_stack;
    fp_t                fc_fp;

    fcontext_t() :
        fc_greg(),
        fc_stack(),
        fc_fp()
    {}
};


/*
* 从 ofc 切换到 nfc 的上下文
* */
intptr_t jump_fcontext(fcontext_t * ofc, fcontext_t nfc,intptr_t vp, bool preserve_fpu = false);

/*
* 创建上下问对象
* */
fcontext_t make_fcontext(void* stack, std::size_t size, fn_t fn);


struct StackTraits
{
    static stack_malloc_fn_t& MallocFunc();

    static stack_free_fn_t& FreeFunc();

    // 获取当前栈顶设置的保护页的页数
    static int & GetProtectStackPageSize();

    // 对保护页的内容做保护
    static bool ProtectStack(void* stack, std::size_t size, int pageSize);

    // 取消对保护页的内存保护，析构是才会调用
    static void UnprotectStack(void* stack, int pageSize);
};
