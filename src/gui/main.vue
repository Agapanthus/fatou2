<template>
    <v-app>
        <v-navigation-drawer permanent app :width="naviWidth" ref="drawer">
            <v-list-item>
                <v-list-item-content>
                    <v-list-item-title class="text-h6"> Templates </v-list-item-title>
                    <v-list-item-subtitle> choose a template </v-list-item-subtitle>
                </v-list-item-content>
            </v-list-item>
            <!--
            <v-divider></v-divider>

            <v-list dense nav>
                <v-list-item v-for="item in items" :key="item.title" link>
                    <v-list-item-icon>
                        <v-icon>{{ item.icon }}</v-icon>
                    </v-list-item-icon>

                    <v-list-item-content>
                        <v-list-item-title>{{ item.title }}</v-list-item-title>
                    </v-list-item-content>
                </v-list-item>
            </v-list>
-->
            <v-divider></v-divider>

            <v-list dense>
                <v-list-item>
                    <v-list-item-content>
                        <v-list-item-title class="text-h6"> Parameters </v-list-item-title>
                    </v-list-item-content>
                </v-list-item>

                <v-list-item>
                    <v-list-item-content>
                        x: {{ dynamicFormatZ(getX) }} <br />
                        y: {{ dynamicFormatZ(getY) }} <br />
                        z: {{ formattedZ }}
                    </v-list-item-content>
                </v-list-item>
            </v-list>

            <v-divider></v-divider>

            <v-list dense>
                <v-list-item>
                    <v-list-item-content>
                        <v-list-item-title class="text-h6"> Renderer </v-list-item-title>
                    </v-list-item-content>
                </v-list-item>

                <v-list-item>
                    <v-list-item-content>
                        <v-progress-linear v-model="progress"></v-progress-linear>
                        Target: {{ rendererTargetRes }} <br />FPS: {{ fps }}<br />Progress:
                        {{ progress }}% (level {{ rendererCurrLevel }})<br />
                        Speed: {{ sps }}<br />
                        Delta: {{ renderTime }}ms
                    </v-list-item-content>
                </v-list-item>
            </v-list>

            <v-divider></v-divider>

            <v-list dense>
                <v-list-item>
                    <v-list-item-content>
                        <v-list-item-title class="text-h6"> Presets </v-list-item-title>
                    </v-list-item-content>
                </v-list-item>

                <v-list-item-group v-model="currentPreset" color="primary">
                    <v-list-item v-for="p in getPresets" :key="p.name">
                        <v-list-item-content>
                            <v-list-item-title class="text-h7">
                                {{ p.name }}
                            </v-list-item-title>
                        </v-list-item-content>
                    </v-list-item>
                </v-list-item-group>
            </v-list>
            <v-divider></v-divider>

            <v-list dense>
                <v-list-item>
                    <v-list-item-content>
                        <v-list-item-title class="text-h6"> Devices </v-list-item-title>
                    </v-list-item-content>
                </v-list-item>

                <v-list-item v-for="device in getDevices" :key="device.name">
                    <v-list-item-content>
                        <v-list-item-title class="text-h7">{{ device.name }} </v-list-item-title>
                        <div v-if="device.score == 0">Rejected: {{ device.reason }}</div>
                        <div v-if="device.score != 0">score: {{ device.score }}</div>
                    </v-list-item-content>
                </v-list-item>
            </v-list>

            <v-divider></v-divider>

            <v-list dense>
                <v-list-item>
                    <v-list-item-content>
                        <v-list-item-title class="text-h6"> Monitors </v-list-item-title>
                    </v-list-item-content>
                </v-list-item>

                <v-list-item-group v-model="selectedMonitor" color="primary">
                    <v-list-item v-for="monitor in getMonitors" :key="monitor.handle">
                        <v-list-item-content>
                            <v-list-item-title class="text-h7"
                                >{{ monitor.name }}
                            </v-list-item-title>
                            <!--   <div>Rect: {{ monitor.rect }}</div>
                            <div>Work: {{ monitor.work }}</div>-->
                            <div>effective dpi: {{ monitor["effective dpi"].join(", ") }}</div>
                            <!-- <div>angular dpi: {{monitor["angular dpi"]}}</div>
                            <div>raw dpi: {{monitor["raw dpi"]}}</div>-->
                            <div>frequency: {{ monitor.frequency }}</div>
                            <div>resolution: {{ monitor.resolution.join(", ") }}</div>
                            <div>BPP: {{ monitor.BPP }}</div>
                        </v-list-item-content>
                    </v-list-item>
                </v-list-item-group>
            </v-list>
        </v-navigation-drawer>

        <v-app-bar :collapse="collapsed" absolute color="deep-purple accent-4" dark app>
            <v-app-bar-nav-icon></v-app-bar-nav-icon>

            <v-toolbar-title>fatou</v-toolbar-title>

            <v-spacer></v-spacer>

            <v-checkbox v-model="collapsed" color="white" hide-details></v-checkbox>
        </v-app-bar>

        <!-- Sizes your content based upon application components -->
        <v-main>
            <!-- Provides the application the proper gutter -->
            <div style="width: 100%; height: 100%" id="renderArea" ref="renderArea"></div>
        </v-main>
    </v-app>
</template>
<script lang="ts" src="./main.ts"></script>
