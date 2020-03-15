import imageio

gif_original = 'C:/Users/Administrator/Documents/MATLAB/demo_above_z0_low_resolution.gif'
gif_speed_up = 'foo.gif'

gif = imageio.mimread(gif_original, memtest=False)

imageio.mimwrite(gif_speed_up, gif, fps=100, memtest=False)