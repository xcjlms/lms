import { createRouter, createWebHistory } from 'vue-router'

const routes = [
  {
    path: '/',
    redirect: '/login',
  },
  {
    path: '/login',
    name: 'Login',
    component: () => import('../views/Login.vue'),
  },
  {
    path: '/home/reader',
    name: 'HomeReader',
    component: () => import('../views/HomeReader.vue'),
  },
  {
    path: '/home/admin',
    name: 'HomeAdmin',
    component: () => import('../views/HomeAdmin.vue'),
  },
  {
    path: '/borrow',
    name: 'Borrow',
    component: () => import('../views/Borrow.vue'),
  },
  {
    path: '/back',
    name: 'Back',
    component: () => import('../views/Back.vue'),
  },
  {
    path: '/fine',
    name: 'Fine',
    component: () => import('../views/Fine.vue'),
  },
]

const router = createRouter({
  history: createWebHistory(),
  routes,
})

// 路由守卫：检查登录状态
router.beforeEach((to, from, next) => {
  const user = localStorage.getItem('lms_user')
  if (to.name !== 'Login' && !user) {
    next('/login')
  } else {
    next()
  }
})

export default router
