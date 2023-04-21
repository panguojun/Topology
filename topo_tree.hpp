/*                         【拓扑树】                               *
*                                                                   *
*                   拓扑从图变为树准备造型使用                      *
*                                                                   *
*                   任何图状结构都可以转化为树状结构                *
*                   按照某种规则遍历树可以得到不同的链结构          *
*                   例如：e{e1,e2{ee1,ee2},e3}                      *
*                   一个形状可以形式化为：                          *
*                       shape = {topo, coord, space}                *
*                                                                   *
*                   另外：图对应了方程 树对应了方程的解结构         *
*                                                                   *    
* * * * * * * * * ** * * * * *  示例代码 * * * * * * * * * ** * * * * 

        topoE te =  {"te",{0}, false };
        topoE te1 = { "te1",{0, 1}, false};
        topoE te2 = { "te2",{2, 3}, false};
        topoE te3 = { "te3", {0}, true };
        topoE te4 = { "te4",{1, 2}, false };
        topoE te5 = { "te5",{1, 3}, false };
      
        te.children.emplace_back(&te1);
        te.children.emplace_back(&te3);
        te3.children.emplace_back(&te4);
        te3.children.emplace_back(&te5);
        te.children.push_back(&te2);

        static const vec3 p_set[] = {
            vec3(-1, -1, 1) ,vec3(0.5, 1, 1), 
            vec3(-2, 1.8, 3), vec3(2, 1.8, -3)
        };
        {
            VECLIST e;
            vec3 p0;
            // 沿着左侧遍历
            te.walkL([&e, &p0](topoE* to, real t) {
                coord3 c1(p_set[to->e.start]), c2(p_set[to->e.end]);
                c1.rot(-45, vec3::UZ); c2.rot(45, vec3::UX);
                vec3 p = blender::slerp(c1, c2, t).o;

                if (VECLIST_ADD(e, p)) {
                    if (p0 != vec3::ZERO)
                        ptr(p0, p, 0.01);
                    p0 = p;
                }
            });
            PRINT(e.size() << " = " << COMBINE_STEPS(5,3));
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
        struct edge { int start, end; } e;          // 边对象(叶节点上)
        bool bmutex = false;                        // 子列表是否互斥 

        // 关联不同的约束与参数空间
        Lineiter space = { 0, 1, 5 };
        int cs = 0;

        tedge* parent = 0;                      // 父
        std::vector<tedge*> children;           // 子列表

		tedge() {}

		// 初始化列表构造函数
		tedge(string name_, edge e_, bool bmutex_ = false)
			: name(std::move(name_)), e(std::move(e_)), bmutex(bmutex_)
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
        }

        void walkL(std::function<void(tedge* to, real t)> cb)
        {
            //PRINT("walkdownL " << name << "," << this->e.start << "," << this->e.end);
            if (children.empty()) {
                if (cb)
                {
                   // PRINT("[" << space.min << "," << space.max << "," << space.steps << "]");
                    space.walk([this, &cb](int i, real t) { cb(this, t); });
                }
                return; // 返回
            }
            if (bmutex) // 可以根据CHILDREN是否多余1个来判断
            {
                //PRINT("互斥选 LEFT");
                children.front()->walkL(cb);

            } else {
                for (auto c : children)
                    c->walkL(cb);
            }
        }
        void walkR(std::function<void(tedge* to, real t)> cb)
        {
           // PRINT("walkdownR " << name << "," << this->e.start << "," << this->e.end);
            if (children.empty()) {
                if (cb)
                {
                    PRINT("[" << space.min << "," << space.max << "," << space.steps << "]");
                    space.walk([this, &cb](int i, real t) { cb(this, t); });
                }
                return; // 返回
            }
            if (bmutex) // 可以根据CHILDREN是否多余1个来判断
            {
               // PRINT("互斥选 RIGHT");
                children.back()->walkR(cb);

            } else {
                for (auto c : children)
                    c->walkR(cb);
            }
        }
        static void walkL(tedge* to, std::function<void(tedge* to, int, int)> cb)
        {
           // PRINT("walkdownL " << to->name);
            if (to->children.empty()) {
                if(cb)
                    cb(to, to->e.start, to->e.end);
                return; // 返回
            }
            if (to->bmutex) // 可以根据CHILDREN是否多余1个来判断
            {
                
              //  PRINT("互斥选 LEFT");
                walkL(to->children.front(), cb);
                
            } else {
                for (auto c : to->children)
                    walkL(c, cb);
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