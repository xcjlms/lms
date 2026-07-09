<template>
  <div class="auth-page">
    <div class="hero-panel">
      <div class="hero-content">
        <div class="hero-title">
          <h1>智慧图书管理系统</h1>
          <p>便捷借阅，智能馆藏</p>
        </div>
        <div class="illustration">
          <div class="book-stack">
            <div class="book book-1"></div>
            <div class="book book-2"></div>
            <div class="book book-3"></div>
          </div>
          <div class="book-sparkles">
            <span></span><span></span><span></span>
          </div>
        </div>
      </div>
    </div>

    <div class="form-panel">
      <div class="auth-card">
        <div class="tab-bar">
          <button
            :class="{ active: activeTab === 'login' }"
            @click="switchTab('login')"
            type="button"
          >登录</button>
          <button
            :class="{ active: activeTab === 'register' }"
            @click="switchTab('register')"
            type="button"
          >注册</button>
        </div>

        <div class="form-body">
          <div class="notice" v-if="infoMessage">{{ infoMessage }}</div>
          <div class="error-alert" v-if="loginError">{{ loginError }}</div>

          <form v-if="activeTab === 'login'" @submit.prevent="submitLogin">
            <div class="form-field">
              <label for="login-username">用户名</label>
              <input
                id="login-username"
                v-model="loginForm.username"
                @input="validateLoginField('username')"
                :class="{ invalid: loginErrors.username }"
                placeholder="请输入用户名"
              />
              <small class="error-text" v-if="loginErrors.username">{{ loginErrors.username }}</small>
            </div>

            <div class="form-field password-field">
              <label for="login-password">密码</label>
              <div class="password-input">
                <input
                  id="login-password"
                  :type="showPassword ? 'text' : 'password'"
                  v-model="loginForm.password"
                  @input="validateLoginField('password')"
                  :class="{ invalid: loginErrors.password }"
                  placeholder="请输入密码"
                />
                <button type="button" class="toggle-btn" @click="showPassword = !showPassword">
                  {{ showPassword ? '隐藏' : '显示' }}
                </button>
              </div>
              <small class="error-text" v-if="loginErrors.password">{{ loginErrors.password }}</small>
            </div>

            <div class="form-field">
              <label for="login-role">角色</label>
              <select
                id="login-role"
                v-model="loginForm.role"
                @change="validateLoginField('role')"
                :class="{ invalid: loginErrors.role }"
              >
                <option value="reader">读者</option>
                <option value="admin">管理员</option>
              </select>
              <small class="error-text" v-if="loginErrors.role">{{ loginErrors.role }}</small>
            </div>

            <div class="form-row">
              <label class="checkbox-label">
                <input type="checkbox" v-model="loginForm.remember" />
                记住我
              </label>
            </div>

            <button class="primary-btn" type="submit">登录</button>

            <div class="extra-text">
              没有账号？
              <span class="link" @click="switchTab('register')">去注册</span>
            </div>
          </form>

          <form v-else @submit.prevent="submitRegister">
            <div class="form-field">
              <label for="reg-username">用户名</label>
              <input
                id="reg-username"
                v-model="registerForm.username"
                @input="validateRegisterField('username')"
                :class="{ invalid: registerErrors.username }"
                placeholder="请输入用户名"
              />
              <small class="error-text" v-if="registerErrors.username">{{ registerErrors.username }}</small>
            </div>

            <div class="form-field">
              <label for="reg-email">邮箱（可选）</label>
              <input
                id="reg-email"
                v-model="registerForm.email"
                @input="validateRegisterField('email')"
                :class="{ invalid: registerErrors.email }"
                placeholder="请输入邮箱"
              />
              <small class="error-text" v-if="registerErrors.email">{{ registerErrors.email }}</small>
            </div>

            <div class="form-field">
              <label for="reg-phone">手机号（可选）</label>
              <input
                id="reg-phone"
                v-model="registerForm.phone"
                @input="validateRegisterField('phone')"
                :class="{ invalid: registerErrors.phone }"
                placeholder="请输入手机号"
              />
              <small class="error-text" v-if="registerErrors.phone">{{ registerErrors.phone }}</small>
            </div>

            <div class="form-field password-field">
              <label for="reg-password">密码</label>
              <div class="password-input">
                <input
                  id="reg-password"
                  :type="showRegPassword ? 'text' : 'password'"
                  v-model="registerForm.password"
                  @input="validateRegisterField('password')"
                  :class="{ invalid: registerErrors.password }"
                  placeholder="请输入密码"
                />
                <button type="button" class="toggle-btn" @click="showRegPassword = !showRegPassword">
                  {{ showRegPassword ? '隐藏' : '显示' }}
                </button>
              </div>
              <small class="error-text" v-if="registerErrors.password">{{ registerErrors.password }}</small>
            </div>

            <div class="form-field password-field">
              <label for="reg-confirm-password">确认密码</label>
              <div class="password-input">
                <input
                  id="reg-confirm-password"
                  :type="showRegConfirmPassword ? 'text' : 'password'"
                  v-model="registerForm.confirmPassword"
                  @input="validateRegisterField('confirmPassword')"
                  :class="{ invalid: registerErrors.confirmPassword }"
                  placeholder="请再次输入密码"
                />
                <button type="button" class="toggle-btn" @click="showRegConfirmPassword = !showRegConfirmPassword">
                  {{ showRegConfirmPassword ? '隐藏' : '显示' }}
                </button>
              </div>
              <small class="error-text" v-if="registerErrors.confirmPassword">{{ registerErrors.confirmPassword }}</small>
            </div>

            <button class="primary-btn" type="submit">注册</button>

            <div class="extra-text">
              已有账号？
              <span class="link" @click="switchTab('login')">返回登录</span>
            </div>
          </form>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { reactive, ref, watch } from 'vue'

const activeTab = ref('login')
const loginForm = reactive({
  username: '',
  password: '',
  role: 'reader',
  remember: false,
})
const loginErrors = reactive({ username: '', password: '', role: '' })
const registerForm = reactive({
  username: '',
  email: '',
  phone: '',
  password: '',
  confirmPassword: '',
})
const registerErrors = reactive({
  username: '',
  email: '',
  phone: '',
  password: '',
  confirmPassword: '',
})
const loginError = ref('')
const infoMessage = ref('')
const showPassword = ref(false)
const showRegPassword = ref(false)
const showRegConfirmPassword = ref(false)

const isValidEmail = (value) => {
  return /^\S+@\S+\.\S+$/.test(value)
}

const isValidPhone = (value) => {
  return /^1[3-9]\d{9}$/.test(value)
}

const resetLoginErrors = () => {
  loginErrors.username = ''
  loginErrors.password = ''
  loginErrors.role = ''
}

const resetRegisterErrors = () => {
  registerErrors.username = ''
  registerErrors.email = ''
  registerErrors.phone = ''
  registerErrors.password = ''
  registerErrors.confirmPassword = ''
}

const switchTab = (tab) => {
  activeTab.value = tab
  loginError.value = ''
  infoMessage.value = ''
  resetLoginErrors()
  resetRegisterErrors()
}

const validateLoginField = (field) => {
  if (field === 'username') {
    loginErrors.username = loginForm.username.trim() ? '' : '请输入用户名'
  }
  if (field === 'password') {
    loginErrors.password = loginForm.password ? '' : '请输入密码'
  }
  if (field === 'role') {
    loginErrors.role = ['reader', 'admin'].includes(loginForm.role) ? '' : '请选择角色'
  }
}

const validateRegisterField = (field) => {
  if (field === 'username') {
    registerErrors.username = registerForm.username.trim()
      ? registerForm.username.length < 3
        ? '用户名至少 3 个字符'
        : ''
      : '请输入用户名'
  }
  if (field === 'email') {
    if (!registerForm.email) {
      registerErrors.email = ''
    } else {
      registerErrors.email = isValidEmail(registerForm.email) ? '' : '请输入正确的邮箱格式'
    }
  }
  if (field === 'phone') {
    if (!registerForm.phone) {
      registerErrors.phone = ''
    } else {
      registerErrors.phone = isValidPhone(registerForm.phone) ? '' : '请输入正确手机号'
    }
  }
  if (field === 'password') {
    if (!registerForm.password) {
      registerErrors.password = '请输入密码'
    } else if (registerForm.password.length < 6) {
      registerErrors.password = '密码至少 6 位'
    } else {
      registerErrors.password = ''
    }
  }
  if (field === 'confirmPassword') {
    registerErrors.confirmPassword = registerForm.confirmPassword
      ? registerForm.confirmPassword === registerForm.password
        ? ''
        : '两次输入密码不一致'
      : '请确认密码'
  }
}

const validateLogin = () => {
  validateLoginField('username')
  validateLoginField('password')
  validateLoginField('role')
  return !loginErrors.username && !loginErrors.password && !loginErrors.role
}

const validateRegister = () => {
  validateRegisterField('username')
  validateRegisterField('email')
  validateRegisterField('phone')
  validateRegisterField('password')
  validateRegisterField('confirmPassword')
  return (
    !registerErrors.username &&
    !registerErrors.email &&
    !registerErrors.phone &&
    !registerErrors.password &&
    !registerErrors.confirmPassword
  )
}

const submitLogin = () => {
  loginError.value = ''
  infoMessage.value = ''
  if (!validateLogin()) {
    return
  }
  const username = loginForm.username.trim()
  const password = loginForm.password
  const role = loginForm.role
  const validAdmin = username === 'admin' && password === 'admin' && role === 'admin'
  const validReader = username === 'reader' && password === 'reader' && role === 'reader'

  if (validAdmin || validReader) {
    infoMessage.value = '登录成功，欢迎回来！'
  } else {
    loginError.value = '登录失败：用户名或密码不正确'
  }
}

const submitRegister = () => {
  infoMessage.value = ''
  loginError.value = ''
  if (!validateRegister()) {
    return
  }
  if (registerForm.username.trim().toLowerCase() === 'admin') {
    registerErrors.username = '用户名已存在'
    return
  }
  const savedUsername = registerForm.username.trim()
  resetRegisterErrors()
  registerForm.username = ''
  registerForm.email = ''
  registerForm.phone = ''
  registerForm.password = ''
  registerForm.confirmPassword = ''
  showRegPassword.value = false
  showRegConfirmPassword.value = false
  activeTab.value = 'login'
  infoMessage.value = `注册成功，用户 ${savedUsername} 已创建，请登录` 
}

watch(activeTab, () => {
  loginError.value = ''
  infoMessage.value = ''
})
</script>

<style scoped>
.auth-page {
  display: flex;
  min-height: 100vh;
  background: #f4f8ff;
  color: #111;
}

.hero-panel {
  flex: 6;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, #4f8cff 0%, #77b6ff 100%);
  color: #fff;
  padding: 40px;
}

.hero-content {
  max-width: 520px;
}

.hero-title h1 {
  font-size: 3rem;
  margin-bottom: 16px;
}

.hero-title p {
  font-size: 1.25rem;
  line-height: 1.8;
  opacity: 0.95;
}

.illustration {
  margin-top: 40px;
  display: flex;
  align-items: center;
  justify-content: flex-start;
  position: relative;
  min-height: 220px;
}

.book-stack {
  position: relative;
  width: 180px;
  height: 160px;
}

.book {
  position: absolute;
  width: 160px;
  height: 36px;
  border-radius: 10px;
  background: rgba(255, 255, 255, 0.25);
  box-shadow: 0 15px 35px rgba(0, 0, 0, 0.12);
}

.book-1 { top: 0; left: 0; transform: rotate(-4deg); }
.book-2 { top: 42px; left: 10px; transform: rotate(3deg); }
.book-3 { top: 84px; left: 24px; transform: rotate(-2deg); }

.book-sparkles span {
  position: absolute;
  display: block;
  width: 10px;
  height: 10px;
  background: rgba(255, 255, 255, 0.9);
  border-radius: 50%;
  box-shadow: 0 0 20px rgba(255, 255, 255, 0.9);
}

.book-sparkles span:nth-child(1) { top: 0; left: 150px; }
.book-sparkles span:nth-child(2) { top: 50px; left: 120px; width: 7px; height: 7px; }
.book-sparkles span:nth-child(3) { top: 120px; left: 90px; width: 8px; height: 8px; }

.form-panel {
  flex: 4;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 40px;
}

.auth-card {
  width: 100%;
  max-width: 420px;
  background: #ffffff;
  border-radius: 20px;
  box-shadow: 0 20px 45px rgba(15, 23, 42, 0.12);
  overflow: hidden;
}

.tab-bar {
  display: flex;
  background: #f4f8ff;
}

.tab-bar button {
  flex: 1;
  padding: 18px 0;
  border: none;
  background: transparent;
  font-size: 1rem;
  color: #65748b;
  cursor: pointer;
  transition: color 0.2s ease, background 0.2s ease;
}

.tab-bar button.active {
  color: #1f4bff;
  background: #fff;
  font-weight: 600;
}

.form-body {
  padding: 32px;
}

.notice,
.error-alert {
  margin-bottom: 20px;
  padding: 14px 16px;
  border-radius: 12px;
  font-size: 0.95rem;
}

.notice {
  background: #f0f9ff;
  color: #0d3d62;
  border: 1px solid #cfe6ff;
}

.error-alert {
  background: #ffe9e9;
  color: #b41717;
  border: 1px solid #ffcdcd;
}

.form-field {
  margin-bottom: 18px;
}

.form-field label {
  display: block;
  margin-bottom: 8px;
  font-size: 0.95rem;
  color: #26384f;
}

.form-field input,
.form-field select {
  width: 100%;
  min-height: 44px;
  padding: 10px 14px;
  border-radius: 12px;
  border: 1px solid #d9e2ef;
  background: #fff;
  font-size: 1rem;
  color: #1b2b44;
  outline: none;
  transition: border-color 0.2s ease, box-shadow 0.2s ease;
}

.form-field input:focus,
.form-field select:focus {
  border-color: #8aa5ff;
  box-shadow: 0 0 0 4px rgba(79, 140, 255, 0.12);
}

.invalid {
  border-color: #f56262 !important;
}

.error-text {
  display: block;
  margin-top: 6px;
  color: #d32f2f;
  font-size: 0.85rem;
}

.password-field {
  position: relative;
}

.password-input {
  display: flex;
  align-items: center;
}

.password-input input {
  flex: 1;
}

.toggle-btn {
  margin-left: 10px;
  padding: 0 12px;
  min-height: 44px;
  border: none;
  border-radius: 12px;
  background: #f4f8ff;
  color: #4f6cff;
  cursor: pointer;
  font-size: 0.95rem;
}

.form-row {
  margin-bottom: 18px;
}

.checkbox-label {
  display: inline-flex;
  align-items: center;
  color: #4f5e7f;
  font-size: 0.95rem;
}

.checkbox-label input {
  margin-right: 8px;
  width: 16px;
  height: 16px;
}

.primary-btn {
  width: 100%;
  padding: 14px;
  border: none;
  border-radius: 14px;
  background: linear-gradient(90deg, #3f7dff 0%, #5b9eff 100%);
  color: #fff;
  font-size: 1rem;
  font-weight: 600;
  cursor: pointer;
  transition: transform 0.2s ease, box-shadow 0.2s ease;
}

.primary-btn:hover {
  transform: translateY(-1px);
  box-shadow: 0 10px 25px rgba(63, 125, 255, 0.24);
}

.extra-text {
  margin-top: 16px;
  text-align: center;
  color: #64748b;
  font-size: 0.95rem;
}

.extra-text .link {
  color: #1f4bff;
  cursor: pointer;
}

@media (max-width: 980px) {
  .auth-page {
    flex-direction: column;
  }

  .hero-panel,
  .form-panel {
    flex: none;
    width: 100%;
  }

  .hero-panel {
    padding: 32px 24px;
  }

  .form-panel {
    padding: 24px;
  }
}

@media (max-width: 640px) {
  .hero-title h1 {
    font-size: 2.25rem;
  }

  .form-body {
    padding: 24px;
  }

  .tab-bar button {
    padding: 14px 0;
  }
}
</style>
