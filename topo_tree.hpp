/*                         ����������                                   *
*                                                                       *
*                   ���˴�ͼ��Ϊ��׼������ʹ��                          *
*                                                                       *
*                   �κ�ͼ״�ṹ������ת��Ϊ��״�ṹ                    *
*                   ����ĳ�ֹ�����������Եõ���ͬ�����ṹ              *
*                   ���磺e{e1,e2{ee1,ee2},e3}                          *
*                   һ����״������ʽ��Ϊ��                              *
*                       shape = {topo, coord, space}                    *
*                                                                       *
*                   ���⣺ͼ��Ӧ�˷��� ����Ӧ�˷��̵Ľ�ṹ             *
*                                                                       *    
* * * * * * * * * * ** * * * * ʾ������ * * * * * * * * * * * * * * * * *
* 
            // ��ͨ������
            topoE te1 = { "e1",{1, 2} }; te1.space.steps = 13;
            topoE te2 = { "e2" };
            topoE te21 = { "e21",{3,4} };
            topoE te22 = { "e22", true };
            topoE te221 = { "e221",{4,5}, topoE::eLeft };
            topoE te222 = { "e222",{4,5}, topoE::eRight };
            topoE te23 = { "e23",{5,6} };
            topoE te3 = { "e3",{7, 8} }; te3.space.steps = 13;
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
	vector<tedge> tedges;					        // ���б�
    struct tshape;
    vector<tshape> tshapes;                         // ���б�

	// ------------------------------
	// ��
	// ------------------------------
    struct tedge
    {
        string name;
        struct edge { int start, end; } e;          // �߶���(Ҷ�ڵ���)
        bool bmutex = false;                        // ���б��Ƿ񻥳� 
        enum asymmetry { eNone, eLeft, eRight} lr = eNone;  // ����

        // ������ͬ��Լ��������ռ�
        Lineiter space = { 0, 1, 5 };
        int cs = 0;

        tedge* parent = 0;                          // ��
        std::vector<tedge*> children;               // ���б�

		tedge() {}

		// ��ʼ���б��캯��
		tedge(string name_, edge e_, bool bmutex_ = false)
			: name(std::move(name_)), e(std::move(e_)), bmutex(bmutex_)
		{}
        tedge(string name_, edge e_, asymmetry lr_, bool bmutex_ = false)
            : name(std::move(name_)), e(std::move(e_)), lr(lr_), bmutex(bmutex_)
        {}
        tedge(string name_, bool bmutex_ = false)
            : name(std::move(name_)), bmutex(bmutex_)
        {}
        tedge(edge e_, bool bmutex_ = false)
            : e(std::move(e_)), bmutex(bmutex_)
        {}
		// �ƶ����캯��
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
		// �ƶ���ֵ����
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
        void addchild(tedge* c)
        {
            children.push_back(c);
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
                return; // ����
            }
            if (bmutex) // ���Ը���CHILDREN�Ƿ����1�����ж�
            {
                //PRINT("����ѡ LEFT");
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
                return; // ����
            }
            if (bmutex) // ���Ը���CHILDREN�Ƿ����1�����ж�
            {
               // PRINT("����ѡ RIGHT");
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
                return; // ����
            }
            if (to->bmutex) // ���Ը���CHILDREN�Ƿ����1�����ж�
            {
                
              //  PRINT("����ѡ LEFT");
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
                return; // ����
            }
            if (to->bmutex) // ���Ը���CHILDREN�Ƿ����1�����ж�
            {
               // PRINT("����ѡ RIGHT");
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
	// ����
	// ------------------------------
	struct tsurface
	{
		std::vector<tedge*> tedges;				// ��ʱ�����б�ʽ���˽ṹ
	};
	// ------------------------------
	// ��״
	// ------------------------------
	struct tshape
	{
		std::vector<tsurface*> tsurfaces;	    // �������б�
	};
}
inline bool VECLIST_ADD(VECLIST& e, crvec p) { if (e.empty() || e.back().p != p) { e.push_back(p); return true; } return false; }
inline void topo_edgeL(VECLIST& e, topoE& te, vec3 p_set[])
{
    //color = RED;
    vec3 p0;
    te.walkL([&](topoE* to, real t) {
        vec3 p;
        if (to->lr == topoE::eNone) {
            p = blend(p_set[to->e.start], p_set[to->e.end], t);
        }
        else {
            coord3 c1(p_set[to->e.start]), c2(p_set[to->e.end]);
            p = to->lr == topoE::eLeft ?
                blender::slerp(c1, c2, (c1 + c2) / 2.0f, t).o :
                blender::slerp_conjcopy(c1, c2, (c1 + c2) / 2.0f, t).o;
        }
        if (VECLIST_ADD(e, p)) {
            /* PRINT("(" << e.size() << ") " << to->name);
             pt3d(p, 0.01 * e.size());
             if (p0 != vec3::ZERO)
                 ptr(p0, p, 0.01);*/
            p0 = p;
        }
        });
}
inline void topo_edgeR(VECLIST& e, topoE& te, vec3 p_set[])
{
    //color = BLUE;
    vec3 p0;
    te.walkR([&](topoE* to, real t) {
        vec3 p;
        if (to->lr == topoE::eNone) {
            p = blend(p_set[to->e.start], p_set[to->e.end], t);
        }
        else {
            coord3 c1(p_set[to->e.start]), c2(p_set[to->e.end]);
            p = to->lr == topoE::eLeft ?
                blender::slerp(c1, c2, (c1 + c2) / 2.0f, t).o :
                blender::slerp_conjcopy(c1, c2, (c1 + c2) / 2.0f, t).o;
        }
        if (VECLIST_ADD(e, p)) {
            /*  PRINT("(" << e.size() << ") " << to->name);
              pt3d(p, 0.01 * e.size());
              if (p0 != vec3::ZERO)
                  ptr(p0, p, 0.01);*/
            p0 = p;
        }
        });
}