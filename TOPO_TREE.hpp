/**                         【拓扑树】
*                   
*                   拓扑从图变为树准备造型使用

*                   任何图状结构都可以转化为树状结构
*                   按照某种规则遍历树可以得到不同的链结构
*                   例如：e{e1,e2{ee1,ee2},e3}
*                   一个形状可以形式化为：
*                       shape = {topo, coord, space}

*                   另外：图对应了方程 树对应了方程的解结构
*/
namespace TOPO_TREE 
{	
	//vector<vec3> p_set;						// 点集
	//vector<constraint> cst_list;					// 约束列表

	struct edge_topo;
	vector< edge_topo> edge_list;					// 边列表

	// ------------------------------
	// 边
	// ------------------------------
	struct edge_topo
	{
		string name;
		struct edge { int start, end; } e;          // 边对象(叶节点上)
		bool bmutex = false;                        // 子列表是否互斥 

		// 关联不同的约束与参数空间
		int cs = 0;
		Lineiter space = { 0, 1, 5 };

		edge_topo* parent = 0;                      // 父
		std::vector<edge_topo*> children;           // 子列表

		edge_topo() {}

		// 初始化列表构造函数
		edge_topo(string name_, edge e_, bool bmutex_ = false)
			: name(std::move(name_)), e(std::move(e_)), bmutex(bmutex_)
		{}
		// 移动构造函数
		edge_topo(edge_topo&& other) noexcept
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
		edge_topo& operator=(edge_topo&& other) noexcept {
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
		void operator += (edge_topo* et)
		{
		    children.push_back(et);
		}

		bool is_compound()
		{
		    return !children.empty();
		}
		static void walkdownL(edge_topo* to, std::function<void(crstr, int, int)> cb)
		{
		    PRINT("walkdownL " << to->name);
		    if (to->children.empty()) {
			if(cb)
			    cb(to->name, to->e.start, to->e.end);
			return; // 返回
		    }
		    if (to->bmutex) // 可以根据CHILDREN是否多余1个来判断
		    {

			PRINT("互斥选 LEFT");
			walkdownL(to->children.front(), cb);

		    }
		    else {
			for (auto c : to->children)
			    walkdownL(c, cb);
		    }
		}
		static void walkdownR(edge_topo* to, std::function<void(crstr, int, int)> cb)
		{
		    PRINT("walkdownR " << to->name);
		    if (to->children.empty()) {
			if (cb)
			    cb(to->name, to->e.end, to->e.start);
			return; // 返回
		    }
		    if (to->bmutex) // 可以根据CHILDREN是否多余1个来判断
		    {
			PRINT("互斥选 RIGHT");
			walkdownR(to->children.back(), cb);

		    }
		    else {
			for (auto it = to->children.rbegin(); it != to->children.rend(); ++it) {
			    walkdownR(*it, cb);
			}
		    }
		}
		static void clear(edge_topo* to)
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
	struct surface_topo
	{
		std::vector<edge_topo*> tlist;				// 暂时考虑列表式拓扑结构
	};
	// ------------------------------
	// 形状
	// ------------------------------
	struct topo_shape
	{
		std::vector<surface_topo*> topo_surfs;	        // 拓扑面列表
	};
	std::vector<topo_shape> topo_shapes;

}
