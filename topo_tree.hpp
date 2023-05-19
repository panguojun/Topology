/*                         【拓扑树】                                   *
*                                                                       *
*                   拓扑从图变为树准备造型使用                          *
*                                                                       *
*                   任何图状结构都可以转化为树状结构                    *
*                   按照某种规则遍历树可以得到不同的链结构              *
*                   例如：e{e1,e2{ee1,ee2},e3}                          *
*                   一个形状可以形式化为：                              *
*                       shape = {topo, coord, space}                    *
*                                                                       *
*                   另外：图对应了方程 树对应了方程的解结构             *
*                                                                       *    
* * * * * * * * * * ** * * * * 示例代码 * * * * * * * * * * * * * * * * *
* 
            // 三通拓扑: te1->te2->te3 管口平行边，其中te2跟一个孔(te22)相连接
            topoE te1 =     { "e1",{1, 2} }; te1.space.steps = 13;
            topoE te2 =     { "e2" };
            topoE te21 =    { "e21",{3,4} };
            topoE te22 =    { "e22", true };
            topoE te221 =   { "e221",{4,5}, topoE::eLeft };
            topoE te222 =   { "e222",{4,5}, topoE::eRight };
            topoE te23 =    { "e23",{5,6} };
            topoE te3 =     { "e3",{7, 8} }; te3.space.steps = 13;
            {
                te22.addchild(&te221);
                te22.addchild(&te222);
            
                te2.addchild(&te21);
                te2.addchild(&te22);
                te2.addchild(&te23);
            }
*/
namespace TOPO_TREE 
{	
	struct tedge;
	vector<tedge> tedges;					        // 边列表
    struct tshape;
    vector<tshape> tshapes;                         // 形列表

	// ------------------------------
	// 边
	// ------------------------------
    struct tedge
    {
        string name;
        struct edge { int start = 0, end = 0; } e;          // 边对象(叶节点上)
        bool bmutex = false;                        // 子列表是否互斥 
       // enum asymmetry { eNone, eLeft, eRight} lr = eNone;  // 手性

        // 关联不同的约束与参数空间
        Lineiter space = { 0, 1, 5 };
        int cs = 0;

        tedge* parent = 0;                          // 父
        std::vector<tedge*> children;               // 子列表

		tedge() {}

		// 初始化列表构造函数
		tedge(string name_, edge e_, bool bmutex_ = false)
			: name(std::move(name_)), e(std::move(e_)), bmutex(bmutex_)
		{}
        tedge(string name_, bool bmutex_ = false)
            : name(std::move(name_)), bmutex(bmutex_)
        {}
        tedge(edge e_, bool bmutex_ = false)
            : e(std::move(e_)), bmutex(bmutex_)
        {}
		// 移动构造函数
		tedge(tedge&& other) noexcept
			: name(std::move(other.name)),
			e(std::move(other.e)),
			bmutex(other.bmutex),
			space(std::move(other.space)),
			parent(other.parent),
			children(std::move(other.children))
		{
			other.parent = nullptr;
		}
		// 移动赋值函数
		tedge& operator=(tedge&& other) noexcept {
			if (this != &other) {
				name = std::move(other.name);
				e = std::move(other.e);
				bmutex = other.bmutex;
				space = std::move(other.space);
				parent = other.parent;
				children = std::move(other.children);
				other.parent = nullptr;
			}
			return *this;
		}
        bool is_compound()
        {
            return !children.empty();
        }
        void operator += (tedge* et)
        {
            children.push_back(et);
            et->parent = this;
        }
        void addchild(tedge* c)
        {
            children.push_back(c);
            c->parent = this;
        }

        // 沿着左侧遍历，bmutex作为开关控制
        void walkL(std::function<void(tedge* to, int i, real t)> cb)
        {
            //PRINT("walkL " << name << "," << this->e.start << "," << this->e.end);
            if (children.empty()) {
                if (cb)
                {
                    //PRINT("[" << space.min << "," << space.max << "," << space.steps << "]");
                    space.walk([this, &cb](int i, real t) { cb(this, i, t); });
                }
                return; // 返回
            }
            if (bmutex) // 可以根据CHILDREN是否多余1个来判断？
            {
                //PRINT("mutex LEFT");
                children.front()->walkL(cb);

            } else {
                for (auto c : children)
                    c->walkL(cb);
            }
        }
        static void walkL(tedge* to, std::function<void(tedge* to, int, int)> cb)
        {
            // PRINT("walkL " << to->name);
            if (to->children.empty()) {
                if (cb)
                    cb(to, to->e.start, to->e.end);
                return; // 返回
            }
            if (to->bmutex) // 可以根据CHILDREN是否多余1个来判断
            {
                //  PRINT("互斥选 LEFT");
                walkL(to->children.front(), cb);

            }
            else {
                for (auto c : to->children)
                    walkL(c, cb);
            }
        }

        // 沿着右侧遍历，bmutex作为开关控制
        void walkR(std::function<void(tedge* to, int i, real t)> cb)
        {
           // PRINT("walkdownR " << name << "," << this->e.start << "," << this->e.end);
            if (children.empty()) {
                if (cb)
                {
                    PRINT("[" << space.min << "," << space.max << "," << space.steps << "]");
                    space.walk([this, &cb](int i, real t) { cb(this, i, t); });
                }
                return; // 返回
            }
            if (bmutex) // 可以根据CHILDREN是否多余1个来判断？
            {
               // PRINT("互斥选 RIGHT");
                children.back()->walkR(cb);

            } else {
                for (auto c : children)
                    c->walkR(cb);
            }
        }
        static void walkR(tedge* to, std::function<void(tedge* to, int, int)> cb)
        {
            //PRINT("walkdownR " << to->name);
            if (to->children.empty()) {
                if (cb)
                    cb(to, to->e.end, to->e.start);
                return; // 返回
            }
            if (to->bmutex) // 可以根据CHILDREN是否多余1个来判断
            {
               // PRINT("互斥选 RIGHT");
                walkR(to->children.back(), cb);

            } else {
                for (auto it = to->children.rbegin(); it != to->children.rend(); ++it) {
                    walkR(*it, cb);
                }
            }
        }
        void clear(tedge* to)
        {
            for (auto c : to->children) {
                clear(c);
            }
            delete to;
        }
        void clear()
        {
            for (auto c : children)
                clear(c);
        }
    };
	// ------------------------------
	// 曲面
	// ------------------------------
	struct tsurface
	{
		std::vector<tedge*> tedges;				// 暂时考虑列表式拓扑结构
	};
	// ------------------------------
	// 形状
	// ------------------------------
	struct tshape
	{
		std::vector<tsurface*> tsurfaces;	    // 拓扑面列表
	};
}