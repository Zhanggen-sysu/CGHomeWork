# CG_HW8

## 实现思路

- 获取鼠标点击的位置

  ```c++
  void CursorPosCallback(GLFWwindow* window, double x, double y) {
  	mouseX = x;
  	mouseY = y;
  }
  
  // main函数调用
  
  glfwSetCursorPosCallback(window, CursorPosCallback);
  
  ```

- 左键添加点，右键删除点

  ```c++
  void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  
  	// 点击左键
  
  	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
          
  		float xpos = mouseX / SCREEN_WIDTH * 2 - 1;
  		float ypos = -(mouseY / SCREEN_HEIGHT * 2 - 1);
  		
  		// 添加控制点
  
  		controlPoints[controlPointsNum * 2] = xpos;
  		controlPoints[controlPointsNum * 2 + 1] = ypos;
  		controlPointsNum++;
  		cout << xpos << ' ' << ypos << endl;
  	}
  
  	// 点击右键
  
  	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT) {
  		
  		// 删除最后添加的控制点
  
  		if (controlPointsNum > 0) {
  			controlPointsNum--;
  		}
  	}
  }
  
  // main函数调用
  
  glfwSetMouseButtonCallback(window, MouseButtonCallback);
  ```

- 伯恩斯坦基函数（PPT 60）

  ```c++
  float Bernstein(int i, int n, float t) {
  
      // 函数中的三个阶乘
  	double Nn = 1, Ni = 1, Nni = 1;
  	for (int k = 2; k <= i; k++) {
  		Ni *= k;
  	}
  
  	for (int k = 2; k <= n - i; k++) {
  		Nni *= k;
  	}
  
  	for (int k = 2; k <= n; k++) {
  		Nn *= k;
  	}
  
  	return Nn * pow(t, i) * pow(1 - t, n - i) / (Ni * Nni);
  }
  
  ```

- 贝塞尔曲线算法（调和函数）（PPT 60）

  ```c++
  void BezierCurve() {
  	curvePointsNum = 0;
  	for (float t = 0.0; t < 1.0; t += 0.001) {
  
  		float tmpX = 0.0f;
  		float tmpY = 0.0f;
  
  		for (int i = 0; i <= controlPointsNum - 1; i++) {
  			float bernstein = Bernstein(i, controlPointsNum - 1, t);
  			tmpX += controlPoints[2 * i] * bernstein;
  			tmpY += controlPoints[2 * i + 1] * bernstein;
  		}
  		curvePoints[curvePointsNum * 2] = tmpX;
  		curvePoints[curvePointsNum * 2 + 1] = tmpY;
  		curvePointsNum++;
  	}
  }
  ```

- 插值方法获取辅助线段端点

  ```c++
  void Lines(float t){
  
  	// 添加控制点
  
  	for (int i = 0; i < controlPointsNum * 2; i++) {
  		linePoints[i] = controlPoints[i];
  	}
  
  	// 插值
  
  	for (int i = controlPointsNum; i > 1; i--) {
  		for (int j = 0; j < i - 1; j++) {
  			linePoints[j * 2] = linePoints[j * 2] * (1 - t) + linePoints[(j + 1) * 2] * t;
  			linePoints[j * 2 + 1] = linePoints[j * 2 + 1] * (1 - t) + linePoints[(j + 1) * 2 + 1] * t;
  		}
          
          // 画出所有t值时的辅助线段
          
  		glBindVertexArray(linesVAO);
  
  		glBindBuffer(GL_ARRAY_BUFFER, VBO);
  
  		glBufferData(GL_ARRAY_BUFFER, sizeof(linePoints), linePoints, GL_STATIC_DRAW);
  
  		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0));
  
  		glEnableVertexAttribArray(0);
  
  		glPointSize(8);
  		glDrawArrays(GL_POINTS, 0, i - 1);
  		glDrawArrays(GL_LINE_STRIP, 0, i - 1);
  	}
  }
  ```

- 着色器很基础，忽略掉...

- 渲染：由于点的数据可能会发生改变，所以数据绑定需要放在渲染循环内部。也比较容易，注意控制点数量不为0时，画点；大于等于2时，画线将控制点连接起来，并且调用BezierCurve()函数获得贝塞尔曲线经过点的坐标并画出来。对于过程中的辅助线段，为了表现出演示效果，设置变量tmp随时间在区间[0, 1]中递增，增速0.001，调用函数Lines(tmp);将线段画出。

  

## 实现效果见视频

